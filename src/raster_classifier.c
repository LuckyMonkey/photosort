#define _POSIX_C_SOURCE 200809L
#include "raster_classifier.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIDE 768U
#define HIST_SIZE 4096U

struct stats {
    unsigned width, height;
    double flat, zero, near, dominant, photo_tiles, graphic_tiles, mixed_tiles, score;
    const char *label, *reason;
};

static int quote(char *out, size_t cap, const char *path)
{
    size_t n = 0U;
    if (cap < 3U) return -1;
    out[n++] = '\'';
    while (*path != '\0') {
        if (*path == '\'') {
            if (n + 4U >= cap) return -1;
            out[n++] = '\''; out[n++] = '\\'; out[n++] = '\''; out[n++] = '\'';
        } else {
            if (n + 2U >= cap) return -1;
            out[n++] = *path;
        }
        ++path;
    }
    out[n++] = '\''; out[n] = '\0';
    return 0;
}

static unsigned y(const unsigned char *p)
{
    return (77U * p[0] + 150U * p[1] + 29U * p[2] + 128U) >> 8U;
}

static double clamp(double v)
{
    return v < 0.0 ? 0.0 : v > 1.0 ? 1.0 : v;
}

static void analyze(const unsigned char *rgb, unsigned width, unsigned height, struct stats *s)
{
    unsigned hist[HIST_SIZE] = {0U};
    unsigned long long neighbors = 0U, zero = 0U, near = 0U, flat = 0U;
    double gradient_sum = 0.0, gradient_sq = 0.0;
    unsigned photo_tiles = 0U, graphic_tiles = 0U, mixed_tiles = 0U;
    for (unsigned row = 0U; row < height; ++row) {
        for (unsigned col = 0U; col < width; ++col) {
            const unsigned char *p = rgb + ((size_t)row * width + col) * 3U;
            ++hist[((p[0] >> 4U) << 8U) | ((p[1] >> 4U) << 4U) | (p[2] >> 4U)];
            if (col + 1U < width) {
                unsigned a = y(p), b = y(p + 3U), d = a > b ? a - b : b - a;
                ++neighbors; if (d == 0U) ++zero; if (d <= 2U) ++near;
                gradient_sum += d; gradient_sq += (double)d * d;
            }
            if (row + 1U < height) {
                unsigned a = y(p), b = y(p + (size_t)width * 3U), d = a > b ? a - b : b - a;
                ++neighbors; if (d == 0U) ++zero; if (d <= 2U) ++near;
                gradient_sum += d; gradient_sq += (double)d * d;
            }
        }
    }
    for (unsigned by = 0U; by < height; by += 8U) for (unsigned bx = 0U; bx < width; bx += 8U) {
        unsigned count = 0U; double sum = 0.0, squares = 0.0;
        for (unsigned row = by; row < height && row < by + 8U; ++row) {
            for (unsigned col = bx; col < width && col < bx + 8U; ++col) {
                double v = y(rgb + ((size_t)row * width + col) * 3U);
                sum += v; squares += v * v; ++count;
            }
        }
        if (count > 0U && squares / count - (sum / count) * (sum / count) <= 4.0) ++flat;
    }
    for (unsigned tile_y = 0U; tile_y < 4U; ++tile_y) for (unsigned tile_x = 0U; tile_x < 4U; ++tile_x) {
        unsigned x0 = tile_x * width / 4U, x1 = (tile_x + 1U) * width / 4U;
        unsigned y0 = tile_y * height / 4U, y1 = (tile_y + 1U) * height / 4U;
        unsigned long long total = 0U, tile_zero = 0U, tile_near = 0U;
        for (unsigned row = y0; row < y1; ++row) for (unsigned col = x0; col < x1; ++col) {
            const unsigned char *p = rgb + ((size_t)row * width + col) * 3U;
            if (col + 1U < x1) { unsigned a = y(p), b = y(p + 3U), d = a > b ? a - b : b - a; ++total; if (!d) ++tile_zero; if (d <= 2U) ++tile_near; }
            if (row + 1U < y1) { unsigned a = y(p), b = y(p + (size_t)width * 3U), d = a > b ? a - b : b - a; ++total; if (!d) ++tile_zero; if (d <= 2U) ++tile_near; }
        }
        double graphic = total == 0U ? 0.5 : 0.5 * (double)tile_zero / total + 0.5 * (double)tile_near / total;
        if (graphic >= 0.62) ++graphic_tiles; else if (graphic <= 0.28) ++photo_tiles; else ++mixed_tiles;
    }
    unsigned max_color = 0U;
    for (unsigned i = 0U; i < HIST_SIZE; ++i) if (hist[i] > max_color) max_color = hist[i];
    unsigned long long pixels = (unsigned long long)width * height;
    double zero_ratio = neighbors ? (double)zero / neighbors : 0.0;
    double near_ratio = neighbors ? (double)near / neighbors : 0.0;
    double mean = neighbors ? gradient_sum / neighbors : 0.0;
    double variance = neighbors ? gradient_sq / neighbors - mean * mean : 0.0;
    double breadth = clamp(mean / 24.0 + (variance > 0.0 ? sqrt(variance) : 0.0) / 48.0);
    s->width = width; s->height = height;
    s->flat = (double)flat / (((width + 7U) / 8U) * ((height + 7U) / 8U));
    s->zero = zero_ratio; s->near = near_ratio;
    s->dominant = pixels ? (double)max_color / pixels : 1.0;
    s->photo_tiles = (double)photo_tiles / 16.0; s->graphic_tiles = (double)graphic_tiles / 16.0; s->mixed_tiles = (double)mixed_tiles / 16.0;
    s->score = clamp(0.25 * breadth + 0.20 * (1.0 - near_ratio) + 0.18 * (1.0 - s->flat) + 0.15 * (1.0 - s->dominant) + 0.22 * s->photo_tiles);
    if (s->score >= 0.78 && s->graphic_tiles < 0.20) { s->label = "PHOTOGRAPH"; s->reason = "broad gradients and photographic tiles"; }
    else if (s->score >= 0.50 && s->graphic_tiles >= 0.10) { s->label = "PHOTO_WITH_GRAPHICS"; s->reason = "photographic content with graphic tiles"; }
    else if (s->score <= 0.25 && s->graphic_tiles >= 0.65) { s->label = "IMAGE"; s->reason = "flat/equal regions and graphic tiles dominate"; }
    else { s->label = "UNCERTAIN"; s->reason = "features do not agree strongly enough"; }
}

int photosort_raster_classify_file(const char *path, FILE *report)
{
    char q[4096], command[8192], header[64];
    unsigned width = 0U, height = 0U;
    FILE *process;
    size_t bytes, received;
    unsigned char *rgb;
    struct stats s;
    int status;

    if (quote(q, sizeof q, path) != 0) return -1;
    (void)snprintf(command, sizeof command,
                   "convert %s -auto-orient -resize '768x768>' "
                   "-print '%%wx%%h\\n' -depth 8 RGB:- 2>/dev/null", q);
    process = popen(command, "r");
    if (process == NULL || fgets(header, sizeof header, process) == NULL ||
        sscanf(header, "%ux%u", &width, &height) != 2U ||
        width == 0U || height == 0U || width > MAX_SIDE || height > MAX_SIDE) {
        if (process != NULL) (void)pclose(process);
        return -1;
    }
    bytes = (size_t)width * height * 3U;
    rgb = malloc(bytes);
    if (rgb == NULL) {
        (void)pclose(process);
        return -1;
    }
    received = fread(rgb, 1U, bytes, process);
    status = pclose(process);
    if (received != bytes || status != 0) {
        free(rgb);
        return -1;
    }
    analyze(rgb, width, height, &s);
    free(rgb);
    fprintf(report,
            ",\"raster_status\":\"ok\",\"width\":%u,\"height\":%u,"
            "\"classification\":\"%s\",\"suggested_folder\":\"%s\","
            "\"photo_score\":%.4f,\"flat_block_ratio\":%.4f,"
            "\"gradient_zero_ratio\":%.4f,\"near_neighbor_ratio\":%.4f,"
            "\"dominant_color_ratio\":%.4f,\"photographic_tile_ratio\":%.4f,"
            "\"graphic_tile_ratio\":%.4f,\"mixed_tile_ratio\":%.4f,\"reason\":\"%s\"",
            s.width, s.height, s.label,
            !strcmp(s.label, "IMAGE") ? "images/other" :
            !strcmp(s.label, "PHOTOGRAPH") ? "photographs" : "images/sort",
            s.score, s.flat, s.zero, s.near, s.dominant,
            s.photo_tiles, s.graphic_tiles, s.mixed_tiles, s.reason);
    return 0;
}
