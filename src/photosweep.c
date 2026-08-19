#define _XOPEN_SOURCE 700
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static const char *root, *out, *mode;
static FILE *report;
static int files;

static int image(const char *p) {
    const char *e = strrchr(p, '.');
    if (!e) return 0;
    return !strcasecmp(e,".jpg") || !strcasecmp(e,".jpeg") || !strcasecmp(e,".png") ||
           !strcasecmp(e,".webp") || !strcasecmp(e,".gif") || !strcasecmp(e,".heic") ||
           !strcasecmp(e,".heif") || !strcasecmp(e,".tif") || !strcasecmp(e,".tiff") ||
           !strcasecmp(e,".bmp") || !strcasecmp(e,".avif");
}

static void jsonq(FILE *f, const char *s) {
    fputc('"', f);
    for (; *s; s++) { if (*s=='"' || *s=='\\') fputc('\\',f); if (*s=='\n') {fputs("\\n",f); continue;} fputc(*s,f); }
    fputc('"', f);
}

static int command(const char *cmd, char *buf, size_t n) {
    FILE *p = popen(cmd, "r"); if (!p) return -1;
    size_t got=fread(buf,1,n-1,p); buf[got]=0; int rc=pclose(p); return rc ? -1 : 0;
}

static void shellq(char *dst, size_t n, const char *s) {
    size_t j=0; dst[j++]='\''; for (; *s && j+4<n; s++) { if (*s=='\'') {dst[j++]='\'';dst[j++]='\\';dst[j++]='\'';dst[j++]='\'';} else dst[j++]=*s; } dst[j++]='\''; dst[j]=0;
}

static void common(FILE *f, const char *p, char *hash) {
    char q[PATH_MAX*2], cmd[PATH_MAX*3], meta[4096]; struct stat st;
    shellq(q,sizeof q,p); snprintf(cmd,sizeof cmd,"sha256sum %s 2>/dev/null",q); hash[0]=0;
    if (command(cmd,meta,sizeof meta)==0) sscanf(meta,"%64s",hash);
    stat(p,&st); fprintf(f,"{\"path\":"); jsonq(f,p); fprintf(f,",\"size\":%lld,\"sha256\":",(long long)st.st_size); jsonq(f,hash);
}

static void sweep_file(const char *p) {
    char q[PATH_MAX*2], cmd[PATH_MAX*3], buf[8192], hash[65];
    shellq(q,sizeof q,p); common(report,p,hash);
    if (!strcmp(mode,"gps")) {
        snprintf(cmd,sizeof cmd,"exiftool -s3 -n -GPSLatitude -GPSLongitude -GPSAltitude -GPSDateTime %s 2>/dev/null",q);
        if (command(cmd,buf,sizeof buf)==0 && *buf) { fprintf(report,",\"status\":\"found\",\"gps\":"); jsonq(report,buf); }
        else fputs(",\"status\":\"none\",\"gps\":null",report);
    } else if (!strcmp(mode,"ocr")) {
        snprintf(cmd,sizeof cmd,"tesseract %s stdout -l eng 2>/dev/null",q);
        if (command(cmd,buf,sizeof buf)==0) { fputs(",\"status\":\"ok\",\"text\":",report); jsonq(report,buf); }
        else fputs(",\"status\":\"unavailable\",\"text\":\"\"",report);
    } else if (!strcmp(mode,"faces")) {
        snprintf(cmd,sizeof cmd,"face-detect %s 2>/dev/null",q);
        if (command(cmd,buf,sizeof buf)==0) { fputs(",\"status\":\"ok\",\"detector\":",report); jsonq(report,buf); }
        else fputs(",\"status\":\"unavailable\",\"facespresent\":null,\"facescount\":null",report);
    } else if (!strcmp(mode,"swatch")) {
        snprintf(cmd,sizeof cmd,"convert %s -resize 1x1! -format '%%[pixel:p{0,0}]' info: 2>/dev/null",q);
        if (command(cmd,buf,sizeof buf)==0 && *buf) { fputs(",\"status\":\"ok\",\"swatch\":",report); jsonq(report,buf); }
        else fputs(",\"status\":\"unavailable\",\"swatch\":null",report);
    }
    fputs("}\n",report); files++;
}

static int walker(const char *p, const struct stat *s, int type, struct FTW *ftw) {
    (void)s; (void)ftw; if (type==FTW_F && image(p)) sweep_file(p); return 0;
}

static int serve(const char *report_path, int port) {
    int srv=socket(AF_INET,SOCK_STREAM,0), one=1; struct sockaddr_in a={0}; char buf[8192];
    if (srv<0) return 1;
    setsockopt(srv,SOL_SOCKET,SO_REUSEADDR,&one,sizeof one); a.sin_family=AF_INET; a.sin_port=htons(port); a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    if (bind(srv,(struct sockaddr*)&a,sizeof a)<0 || listen(srv,8)<0) return 1;
    fprintf(stderr,"duplicate chooser: http://127.0.0.1:%d/\n",port);
    for (;;) { int c=accept(srv,0,0); if(c<0) continue; int n=read(c,buf,sizeof buf-1); if(n<0){close(c);continue;} buf[n]=0;
        if (!strncmp(buf,"POST /decision",14)) { FILE *d=fopen("duplicate-decisions.jsonl","a"); if(d){ char *body=strstr(buf,"\r\n\r\n"); if(body) fprintf(d,"%s\n",body+4); fclose(d);} dprintf(c,"HTTP/1.1 204 No Content\r\n\r\n"); }
        else { FILE *r=fopen(report_path,"r"); dprintf(c,"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n<h1>PhotoSweep duplicate chooser</h1><p>Review only; nothing is deleted automatically.</p><pre>"); if(r){while(fgets(buf,sizeof buf,r)) { for(char *x=buf;*x;x++){ if(*x=='&') dprintf(c,"&amp;"); else if(*x=='<') dprintf(c,"&lt;"); else if(*x=='>') dprintf(c,"&gt;"); else (void)!write(c,x,1); }} fclose(r);} dprintf(c,"</pre><p>Decisions can be POSTed to /decision as JSON.</p>"); }
        close(c);
    }
}

int main(int argc, char **argv) {
    if (argc>=2 && !strcmp(argv[1],"chooser")) { if(argc<3){fprintf(stderr,"usage: photosweep chooser REPORT [PORT]\n");return 2;} return serve(argv[2],argc>3?atoi(argv[3]):8765); }
    if (argc<5 || strcmp(argv[1],"run")) { fprintf(stderr,"usage: photosweep run TYPE ROOT OUT\n       TYPE: ocr | faces | gps | swatch | all\n       photosweep chooser REPORT [PORT]\n"); return 2; }
    mode=argv[2]; root=argv[3]; out=argv[4]; (void)out; const char *requested=mode;
    mkdir(out,0755); const char *types[] = {"ocr","faces","gps","swatch"};
    for(int i=0;i<4;i++) { if(strcmp(requested,"all") && strcmp(requested,types[i])) continue; mode=types[i]; char path[PATH_MAX]; snprintf(path,sizeof path,"%s/%s.jsonl",out,mode); report=fopen(path,"a"); if(!report){perror(path);return 1;} files=0; if(nftw(root,walker,20,FTW_PHYS)<0){perror(root);return 1;} fclose(report); printf("%s: %d records -> %s\n",mode,files,path); }
    return 0;
}
