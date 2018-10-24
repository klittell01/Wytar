
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

int tArchive(FILE * fd, struct t_header ** arc,
    const int fc, const char * files[]){
        struct t_header ** myTar = arc;
        int rtn1 = tWrite(fd, myTar, arc, fc, files);
        printf("my return value: %d\n", rtn1 );
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

int tWrite(FILE * fd, struct t_header ** top, struct t_header ** arc,
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
        // meta data writing part goes here
        if((*myTar) -> typeflag == DIRTYPE){
            int len = strlen((*myTar) -> name);
            char * parent = calloc(len + 1, sizeof(char));
            strncpy(parent, (*myTar) -> name, len);

            // add a '/' character to the end
            if ((len < 99)){
                (*myTar) -> name[len] = '/';
                (*myTar) -> name[len + 1] = '\0';
                // TODO calculate checksum
            }

            // go through directory
            DIR *dp = opendir((*myTar) -> name);
            if (!dp){
                perror("Error: Cannot read directory");
            }
            printf("filetype is: %c\n", (*myTar) -> typeflag);
            printf("directory recognized\n");

            struct dirent *ep;
            while ((ep = readdir(dp))){
                if (strcmp(ep -> d_name, ".") == 0){
                    // do nothing
                } else if (strcmp(ep -> d_name, "..") == 0){
                    // do nothing
                } else {
                    char * myPath = malloc(len + strlen(ep -> d_name));
                    sprintf(myPath, "%s/%s", (*myTar) -> name, ep -> d_name);

                    // recursively write each subdirectory
                    struct t_header ** next = malloc(sizeof(struct t_header));
                    int rtn2 = tWrite(fd, next, arc, 1, (const char **) &myPath);
                    if (rtn2 < 0){
                        printf("Error: unable to write data\n");
                    }
                }
            }
        closedir(dp);
        } else {

        // TODO: write_size metadata here should always work since it is 512
            if (((*myTar) -> typeflag == REGTYPE) || ((*myTar) -> typeflag == AREGTYPE) ||
            ((*myTar) -> typeflag == SYMTYPE)){
                // open file to read from
                FILE *fdR = fopen((*myTar) -> name, "r");
                printf("file to read from size: %s\n", (*myTar) -> size);
                if (fdR == NULL){
                    perror("Error");
                    return -1;
                }
                printf("i got past open\n");
                int written = 0;
                int writeCount;
                int rCount = 0;
                int tmp = 0;
                rCount = fread(buff, sizeof(char), 512, fdR);
                while (rCount > 0){
                    printf("read count inside is this %d\n", rCount);
                    tmp ++;
                    writeCount = fwrite(buff, 1, rCount, fd);
                    if(writeCount <= 0){
                        perror("Error: writting");
                    }
                    rCount = fread(buff, sizeof(char), 512, fdR);
                }
                printf("we looped this many times: %d\n", tmp);
            }

            // pad the left over size to fill the block
            unsigned int mySize = atoi((*myTar) -> size);
            int padding = 512 - mySize % 512;
            if (padding < 512){
                int j;
                printf("padding is: %d\n", padding);
                for (j = 0; j < padding; j++){
                    int written = 0;
                    int writeCount;
                    int rCount = 0;
                    writeCount = fwrite("\0", 1, 1, fd);
                }
            }
            printf("my size is %u\n", mySize);

            // return 0;
        }
    }

    printf("test two\n");
    return -1;
}

int tWriteEnd(FILE * fd, int offset){
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

    // put name and mode in header
    strcpy(header -> name, filename);
    strcpy(header -> mode, mode);

    unsigned int  timeInt = fileStat.st_mtime;
    char mTimebuf[12];
    sprintf(mTimebuf, "%u", timeInt);
    strcpy(header -> mtime, mTimebuf);
    // get uid and save to header
    struct passwd pwd;
    struct passwd *result;
    char *buf;
    char uidBuff[32];
    size_t bufSize = 182;
    buf = malloc(bufSize);
    getpwnam_r(filename, &pwd, buf, bufSize, &result);
    sprintf(uidBuff, "%ld", (long)pwd.pw_uid);
    strcpy(header -> uid, uidBuff);

    // get gid and save to header
    gid_t grid;
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

    // get file type and save to header
    if(S_ISDIR(fileStat.st_mode) != 0){
        // directory
        header -> typeflag = DIRTYPE;
    } else if(S_ISLNK(fileStat.st_mode) != 0) {
        // link so we dont put a size just leave as zeros
        header -> typeflag = SYMTYPE;
    } else {
        // not a dir or link
        header -> typeflag = REGTYPE;
    }

    // put size in the header
    int mySize = (int) fileStat.st_size;
    printf("my first size is: %d\n", mySize);
    char sizeBuf[12];
    sprintf(sizeBuf, "%d", mySize);
    strcpy(header -> size, sizeBuf);

    // TODO: compute checksum here and put in header
    unsigned int checksum = 0;

    return 0;
}
