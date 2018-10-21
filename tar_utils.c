
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <math.h>
#include "tar_utils.h"

int tArchive(const int fd, struct t_header ** arc,
    const int fc, const char * files[]){
        int rtn1 = tWrite(fd, arc, fc, files);
        if(rtn1 < 0){
            fprintf(stderr, "Error: Failed to write entries\n");
            return -1;
        }
        int offset = 0;
        int rtn2 = tWriteEnd(fd, offset);
        if(rtn2 < 0){
            fprintf(stderr, "Error: Failed to write end blocks\n");
            return -1;
        }
        return 0;
    }

int tWrite(const int fd, struct t_header ** arc,
    const int fc, const char * files[]){
        struct t_header ** myTar = arc;
        char buff[512];
        int i = 0;
        for (i; i < fc; i++){
            *myTar = malloc(sizeof(struct t_header));
            int rtn1 = tStatFile(*myTar, files[i]);
            if(rtn1 < 0){
                fprintf(stderr, "Error: Failed to stat file\n");
                return -1;
            }
            // put file writing part here
        }
        return 0;
    }

int tWriteEnd(const int fd, int offset){
    return -1;
}

int tStatFile(struct t_header * header, const char * filename){
    struct stat fileStat;

    memset(header, 0, sizeof(struct t_header));

    char filePath[1024];
    char tmpName[256];

    strcpy (filePath, "/");
    strcpy(tmpName, filename);
    strcat(filePath,tmpName);

    int rtn1 = lstat(tmpName, &fileStat);
    if (rtn1 < 0){
        fprintf(stderr, "Error: Stat returned error\n");
        return -1;
    }
    char mode[9];
    char tmp[1];

    strcat(mode, ((fileStat.st_mode & S_IRUSR) ? "r" : "-"));
    strcat(mode, ((fileStat.st_mode & S_IWUSR) ? "w" : "-"));
    strcat(mode, ((fileStat.st_mode & S_IXUSR) ? "x" : "-"));
    strcat(mode, ((fileStat.st_mode & S_IRGRP) ? "r" : "-"));
    strcat(mode, ((fileStat.st_mode & S_IWGRP) ? "w" : "-"));
    strcat(mode, ((fileStat.st_mode & S_IXGRP) ? "x" : "-"));
    strcat(mode, ((fileStat.st_mode & S_IROTH) ? "r" : "-"));
    strcat(mode, ((fileStat.st_mode & S_IWOTH) ? "w" : "-"));
    strcat(mode, ((fileStat.st_mode & S_IXOTH) ? "x" : "-"));

    strcpy(header -> name, filename);
    strcpy(header -> mode, mode);

    uid_t uid;
    gid_t grid;

    struct passwd pwd;
    struct passwd *result;
    char *buf;
    char uidBuff[32];
    size_t bufSize = 182;
    buf = malloc(bufSize);
    getpwnam_r(filename, &pwd, buf, bufSize, &result);
    sprintf(uidBuff, "%ld", (long)pwd.pw_uid);
    strcpy(header -> uid, uidBuff);

    char gidBuff[32];
    grid = fileStat.st_gid;
    struct group *gr;
    gr = malloc(182);
    struct group *groupR;
    getgrnam_r(filename, gr, buf, bufSize, &groupR);
    sprintf(gidBuff, "%ld", (long)gr->gr_gid);
    strcpy(header -> gid, gidBuff);

    free(gr);
    free(buf);

    printf("data check: %s\n", header -> gid);
    //strcpy( header -> mode[0], (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    /*
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
    */

    return 0;
}
