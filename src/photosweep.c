/*
 * PhotoSweep: report-first native photo analysis and local duplicate review.
 *
 * The executable deliberately has no mutation path. It walks image files,
 * computes a stable identity, delegates optional analysis to native tools,
 * and writes append-only JSONL. The chooser is loopback-only and records
 * review decisions; it never interprets them as permission to delete files.
 */
#define _XOPEN_SOURCE 700

#include <arpa/inet.h>
#include <errno.h>
#include <ftw.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "raster_classifier.h"

static const char *g_root;
static const char *g_output;
static const char *g_mode;
static FILE *g_report;
static unsigned long g_files;

/* Return nonzero only for image formats allowed by rules/photosweep.yaml. */
static int is_supported_image(const char *path)
{
    const char *extension = strrchr(path, '.');
    if (extension == NULL) {
        return 0;
    }
    return !strcasecmp(extension, ".jpg") || !strcasecmp(extension, ".jpeg") ||
           !strcasecmp(extension, ".png") || !strcasecmp(extension, ".webp") ||
           !strcasecmp(extension, ".gif") || !strcasecmp(extension, ".heic") ||
           !strcasecmp(extension, ".heif") || !strcasecmp(extension, ".tif") ||
           !strcasecmp(extension, ".tiff") || !strcasecmp(extension, ".bmp") ||
           !strcasecmp(extension, ".avif");
}

/* Write a valid JSON string, including all control characters below U+0020. */
static void write_json_string(FILE *stream, const char *text)
{
    const unsigned char *cursor = (const unsigned char *)text;
    fputc('"', stream);
    for (; *cursor != '\0'; ++cursor) {
        switch (*cursor) {
        case '"':
        case '\\':
            fputc('\\', stream);
            fputc(*cursor, stream);
            break;
        case '\b': fputs("\\b", stream); break;
        case '\f': fputs("\\f", stream); break;
        case '\n': fputs("\\n", stream); break;
        case '\r': fputs("\\r", stream); break;
        case '\t': fputs("\\t", stream); break;
        default:
            if (*cursor < 0x20U) {
                fprintf(stream, "\\u%04x", (unsigned)*cursor);
            } else {
                fputc(*cursor, stream);
            }
            break;
        }
    }
    fputc('"', stream);
}

/* Run one fixed external-tool command and capture bounded stdout. */
static int run_command(const char *command, char *output, size_t capacity)
{
    FILE *process;
    size_t bytes;
    int status;

    if (capacity < 2U) {
        return -1;
    }
    process = popen(command, "r");
    if (process == NULL) {
        output[0] = '\0';
        return -1;
    }
    bytes = fread(output, 1U, capacity - 1U, process);
    output[bytes] = '\0';
    status = pclose(process);
    if (status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return -1;
    }
    return 0;
}

/* Quote a path for the POSIX shell boundary used by the fixed tool commands. */
static int shell_quote(char *destination, size_t capacity, const char *path)
{
    size_t position = 0U;

    if (capacity < 3U) {
        return -1;
    }
    destination[position++] = '\'';
    while (*path != '\0') {
        if (*path == '\'') {
            if (position + 4U >= capacity) {
                destination[0] = '\0';
                return -1;
            }
            destination[position++] = '\'';
            destination[position++] = '\\';
            destination[position++] = '\'';
            destination[position++] = '\'';
        } else {
            if (position + 2U >= capacity) {
                destination[0] = '\0';
                return -1;
            }
            destination[position++] = *path;
        }
        ++path;
    }
    destination[position++] = '\'';
    destination[position] = '\0';
    return 0;
}

/* Emit the identity fields shared by every sweep record. */
static int write_common_identity(FILE *stream, const char *path, char *hash)
{
    char quoted_path[PATH_MAX * 2U];
    char command[PATH_MAX * 3U];
    char hash_output[4096];
    struct stat metadata;

    hash[0] = '\0';
    if (stat(path, &metadata) != 0 || shell_quote(quoted_path, sizeof quoted_path, path) != 0) {
        return -1;
    }
    (void)snprintf(command, sizeof command, "sha256sum %s 2>/dev/null", quoted_path);
    if (run_command(command, hash_output, sizeof hash_output) != 0 ||
        sscanf(hash_output, "%64s", hash) != 1) {
        return -1;
    }
    fputs("{\"path\":", stream);
    write_json_string(stream, path);
    fprintf(stream, ",\"size\":%lld,\"sha256\":", (long long)metadata.st_size);
    write_json_string(stream, hash);
    return 0;
}

/* Execute exactly one sweep for one file and append one JSONL record. */
static void sweep_file(const char *path)
{
    char quoted_path[PATH_MAX * 2U];
    char command[PATH_MAX * 3U];
    char result[8192];
    char hash[65];

    if (write_common_identity(g_report, path, hash) != 0) {
        fprintf(g_report, "{\"path\":");
        write_json_string(g_report, path);
        fputs(",\"status\":\"error\",\"error\":\"identity unavailable\"}\n", g_report);
        ++g_files;
        return;
    }
    if (!strcmp(g_mode, "raster")) {
        if (photosort_raster_classify_file(path, g_report) != 0) fputs(",\"raster_status\":\"error\",\"error\":\"raster decode unavailable\"}\n", g_report);
        else { fputc('}', g_report); fputc('\n', g_report); }
        ++g_files;
        return;
    }
    if (shell_quote(quoted_path, sizeof quoted_path, path) != 0) {
        fputs(",\"status\":\"error\",\"error\":\"path too long\"}\n", g_report);
        ++g_files;
        return;
    }

    if (!strcmp(g_mode, "gps")) {
        (void)snprintf(command, sizeof command,
                       "exiftool -s3 -n -GPSLatitude -GPSLongitude -GPSAltitude -GPSDateTime %s 2>/dev/null",
                       quoted_path);
        if (run_command(command, result, sizeof result) == 0 && result[0] != '\0') {
            fputs(",\"status\":\"found\",\"gps\":", g_report);
            write_json_string(g_report, result);
        } else {
            fputs(",\"status\":\"none\",\"gps\":null", g_report);
        }
    } else if (!strcmp(g_mode, "ocr")) {
        (void)snprintf(command, sizeof command, "tesseract %s stdout -l eng 2>/dev/null", quoted_path);
        if (run_command(command, result, sizeof result) == 0) {
            fputs(",\"status\":\"ok\",\"text\":", g_report);
            write_json_string(g_report, result);
        } else {
            fputs(",\"status\":\"unavailable\",\"text\":\"\"", g_report);
        }
    } else if (!strcmp(g_mode, "faces")) {
        (void)snprintf(command, sizeof command, "face-detect %s 2>/dev/null", quoted_path);
        if (run_command(command, result, sizeof result) == 0) {
            fputs(",\"status\":\"ok\",\"detector\":", g_report);
            write_json_string(g_report, result);
        } else {
            fputs(",\"status\":\"unavailable\",\"facespresent\":null,\"facescount\":null", g_report);
        }
    } else if (!strcmp(g_mode, "swatch")) {
        (void)snprintf(command, sizeof command,
                       "convert %s -resize 1x1! -format '%%[pixel:p{0,0}]' info: 2>/dev/null",
                       quoted_path);
        if (run_command(command, result, sizeof result) == 0 && result[0] != '\0') {
            fputs(",\"status\":\"ok\",\"swatch\":", g_report);
            write_json_string(g_report, result);
        } else {
            fputs(",\"status\":\"unavailable\",\"swatch\":null", g_report);
        }
    }
    fputs("}\n", g_report);
    ++g_files;
}

/* nftw callback: reject non-files and policy-excluded extensions. */
static int walk_callback(const char *path, const struct stat *metadata, int type, struct FTW *state)
{
    (void)metadata;
    (void)state;
    if (type == FTW_F && is_supported_image(path)) {
        sweep_file(path);
    }
    return 0;
}

/* Serve escaped reports on loopback and record review decisions only. */
static int serve_chooser(const char *report_path, int port)
{
    int server;
    int reuse = 1;
    struct sockaddr_in address = {0};
    char request[8192];

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket");
        return 1;
    }
    (void)setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
    address.sin_family = AF_INET;
    address.sin_port = htons((uint16_t)port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(server, (struct sockaddr *)&address, sizeof address) != 0 || listen(server, 8) != 0) {
        perror("chooser bind/listen");
        close(server);
        return 1;
    }
    fprintf(stderr, "duplicate chooser: http://127.0.0.1:%d/\n", port);
    for (;;) {
        int client = accept(server, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR) continue;
            continue;
        }
        ssize_t received = read(client, request, sizeof request - 1U);
        if (received > 0) {
            request[received] = '\0';
            if (!strncmp(request, "POST /decision", 15U)) {
                FILE *decisions = fopen("duplicate-decisions.jsonl", "a");
                char *body = strstr(request, "\r\n\r\n");
                if (decisions != NULL) {
                    if (body != NULL) fprintf(decisions, "%s\n", body + 4);
                    fclose(decisions);
                }
                dprintf(client, "HTTP/1.1 204 No Content\r\n\r\n");
            } else {
                FILE *input = fopen(report_path, "r");
                dprintf(client, "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n"
                                "<h1>PhotoSweep duplicate chooser</h1>"
                                "<p>Review only; nothing is deleted automatically.</p><pre>");
                if (input != NULL) {
                    while (fgets(request, sizeof request, input) != NULL) {
                        for (char *cursor = request; *cursor != '\0'; ++cursor) {
                            if (*cursor == '&') dprintf(client, "&amp;");
                            else if (*cursor == '<') dprintf(client, "&lt;");
                            else if (*cursor == '>') dprintf(client, "&gt;");
                            else (void)!write(client, cursor, 1U);
                        }
                    }
                    fclose(input);
                }
                dprintf(client, "</pre><p>POST JSON decisions to /decision.</p>");
            }
        }
        close(client);
    }
}

/* CLI dispatcher: expand all into independent passes and never mutate inputs. */
int main(int argc, char **argv)
{
    const char *requested;
    const char *types[] = {"ocr", "faces", "gps", "swatch", "raster"};
    struct stat root_metadata;

    if (argc >= 2 && !strcmp(argv[1], "chooser")) {
        if (argc < 3) {
            fprintf(stderr, "usage: photosweep chooser REPORT [PORT]\n");
            return 2;
        }
        return serve_chooser(argv[2], argc > 3 ? atoi(argv[3]) : 8765);
    }
    if (argc < 5 || strcmp(argv[1], "run")) {
        fprintf(stderr, "usage: photosweep run TYPE ROOT OUT\n"
                        "       TYPE: ocr | faces | gps | swatch | raster | all\n"
                        "       photosweep chooser REPORT [PORT]\n");
        return 2;
    }
    if (stat(argv[3], &root_metadata) != 0 || !S_ISDIR(root_metadata.st_mode)) {
        fprintf(stderr, "photosweep: root is not a directory: %s\n", argv[3]);
        return 2;
    }
    g_root = argv[3];
    g_output = argv[4];
    requested = argv[2];
    if (mkdir(g_output, 0755) != 0 && errno != EEXIST) {
        perror(g_output);
        return 1;
    }
    for (size_t index = 0U; index < sizeof types / sizeof types[0]; ++index) {
        char report_path[PATH_MAX];
        if (strcmp(requested, "all") != 0 && strcmp(requested, types[index]) != 0) continue;
        g_mode = types[index];
        (void)snprintf(report_path, sizeof report_path, "%s/%s.jsonl", g_output, g_mode);
        g_report = fopen(report_path, "a");
        if (g_report == NULL) {
            perror(report_path);
            return 1;
        }
        g_files = 0U;
        if (nftw(g_root, walk_callback, 20, FTW_PHYS) != 0) {
            perror(g_root);
            fclose(g_report);
            return 1;
        }
        if (fclose(g_report) != 0) {
            perror(report_path);
            return 1;
        }
        printf("%s: %lu records -> %s\n", g_mode, g_files, report_path);
    }
    return 0;
}
