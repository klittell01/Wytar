/*
 * tar_utils.c
 * Author: Kevin Littell
 * Date: 10-19-2018
 * COSC 3750, program 5
 * replacment for the tar utility
 */
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
    if(rtn1 < 0){
        fprintf(stderr, "Error: Failed to write entries\n");
        return -1;
    }
    int rtn2 = tWriteEnd(fd);
    if(rtn2 < 0){
        fprintf(stderr, "Error: Failed to write end blocks\n");
        return -1;
    }
    return 0;
}

int tExtract(FILE * fd, struct t_header ** arc,
    const int fc, const char * files[], const char * tarName){

    //calculate how many files to extract
    struct stat fileStat;
    char filePath[1024];
    strcpy (filePath, "./");
    strcat(filePath,tarName);
    int rtn1 = lstat(filePath, &fileStat);
    if (rtn1 < 0){
        perror("Stat error:");
        return -1;
    }

    fseek(fd, 0, SEEK_SET);
    FILE * fdNew;
    struct t_header ** myTar = arc;
    char buff[512];
    int sizeOfTar = (int) fileStat.st_size;
    int totalBlocks = sizeOfTar / 512;
    int blocksToRead = 0;
    blocksToRead = totalBlocks;
    while (blocksToRead > 2){
        *myTar = malloc(sizeof(struct t_header));
        int rd = fread((*myTar), sizeof(struct t_header), 1, fd);
        if(rd < 0){
            perror("Error: reading");
        }
        blocksToRead--;
        unsigned int mySize = atoi((*myTar) -> size);
        int lastWrite = mySize % 512;
        int eof = ceil(mySize / 512);
        int pathLen = strlen((*myTar) -> name);
        fdNew = fopen((*myTar) -> name, "wb");
        if((*myTar) -> typeflag != DIRTYPE){
            int i;
            for(i = 0; i <= eof; i++){
                if(i < eof){
                    int readCount = fread(buff, 1, 512, fd);
                    blocksToRead--; //decrement the amount we need to still read
                    if(readCount < 0){
                        perror("Error: reading");
                        return -1;
                    }
                    int writeCount = fwrite(buff, 1, readCount, fdNew);
                    if(writeCount < 0){
                        perror("Error: writing");
                    }
                } else {
                    int readCount = fread(buff, 1, lastWrite, fd);
                    blocksToRead--; //decrement the amount we need to still read
                    if(readCount < 0){
                        perror("Error: reading");
                        return -1;
                    }
                    int writeCount = fwrite(buff, 1, readCount, fdNew);
                    if(writeCount < 0){
                        perror("Error: writing");
                    }
                    int seekRtn = fseek(fd, 512 - lastWrite, SEEK_CUR);
                    blocksToRead--; //decrement the amount we need to still read
                    if(seekRtn < 0){
                        perror("Error seeking in file");
                        return -1;
                    }
                }
            }
        } else {
            // must be a directory
            char * path = calloc(pathLen + 1, sizeof(char));
            strcpy(path, (*myTar) -> name);
            // remove last '/'
            if (path[pathLen - 1] ==  '/'){
                path[pathLen - 1] = 0;
            }
            if (mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0){
                fprintf(stderr, "Failed creating directory. Error: %d\n", errno);
                return -1;
            }
            blocksToRead--; //decrement the amount we need to still read
        }
    }
    return 0;
}

int tWrite(FILE * fd, struct t_header ** top, struct t_header ** arc,
    const int fc, const char * files[]){
    struct t_header ** myTar = arc;
    char buff[512];
    int i = 0;

    for (i = 0; i < fc; i++){
        *myTar = malloc(sizeof(struct t_header));
        int rtn1 = tStatFile(*myTar, files[i]);
        if(rtn1 < 0){
            perror("Stat Error");
            continue;
        }
        // meta data writing part goes here
        int metaRtn;

        metaRtn = fwrite((*myTar), sizeof(struct t_header), 1, fd);
        if(metaRtn < 0){
            perror("Error writing in file");
            return -1;
        }


        ///////////////////////////////
        /// if it is a directory //////
        //////////////////////////////
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

            struct dirent *ep;
            while ((ep = readdir(dp))){
                if (strcmp(ep -> d_name, ".") == 0){
                    // do nothing
                } else if (strcmp(ep -> d_name, "..") == 0){
                    // do nothing
                } else {
                    char * myPath = malloc(len + strlen(ep -> d_name));
                    sprintf(myPath, "%s/%s", (*myTar) -> name, ep -> d_name);
                    struct t_header ** next = malloc(sizeof(struct t_header));
                    int rtn2 = tWrite(fd, next, arc, 1, (const char **) &myPath);
                    if (rtn2 < 0){
                        perror("Error writing in file");
                    }
                }
            }
            closedir(dp);
        } else {
            if (((*myTar) -> typeflag == REGTYPE) || ((*myTar) -> typeflag == AREGTYPE) ||
            ((*myTar) -> typeflag == SYMTYPE)){
                // open file to read from
                FILE *fdR = fopen((*myTar) -> name, "rb");
                if (fdR == NULL){
                    perror("Error");
                    return -1;
                }
                int writeCount;
                int rCount = 0;
                int tmp = 0;
                rCount = fread(buff, sizeof(char), 512, fdR);
                while (rCount > 0){
                    tmp ++;
                    writeCount = fwrite(buff, 1, rCount, fd);
                    if(writeCount <= 0){
                        perror("Error: writing");
                    }
                    rCount = fread(buff, sizeof(char), 512, fdR);
                }
                fclose(fdR);
            }

            // pad the left over size to fill the block
            unsigned int mySize = atoi((*myTar) -> size);
            int padding = 512 - mySize % 512;
            if (padding < 512){
                int j;
                for (j = 0; j < padding; j++){
                    int wC;
                    wC = fwrite("\0", 1, 1, fd);
                    if(wC < 0){
                        printf("Error: writing the block padding failed");
                        return -1;
                    }
                }
            }
        }
    }
    int rtn3 = tWriteEnd(fd);
    if(rtn3 < 0){
        printf("Error: writing the two end blocks failed");
        return -1;
    }
    return 0;
}

int tWriteEnd(FILE * fd){
    int j;
    for (j = 0; j < 1024; j++){
        int writeCount;
        writeCount = fwrite("\0", 1, 1, fd);
        if (writeCount <= 0){
            return -1;
        }
    }
    return 0;
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
    char mode[8];
    memset(mode, 0, sizeof(char) * 8);
    sprintf(header -> mode, "%07o", fileStat.st_mode);
    sprintf(header -> uid, "%07o", fileStat.st_uid);
    sprintf(header -> gid, "%07o", fileStat.st_gid);
    sprintf(header -> size, "%011o", (int) fileStat.st_size);

    // put name and mode in header
    strcpy(header -> name, filename);

    // modified time saved to header
    unsigned int  timeInt = fileStat.st_mtime;
    char mTimebuf[12];
    sprintf(mTimebuf, "%u", timeInt);
    strcpy(header -> mtime, mTimebuf);

    // get uname and save to header
    struct passwd pwd;
    struct passwd *result;
    char *buf;
    char uidBuff[32];
    size_t bufSize = 182;
    buf = malloc(bufSize);
    getpwnam_r(filename, &pwd, buf, bufSize, &result);
    sprintf(uidBuff, "%ld", (long)pwd.pw_uid);
    strcpy(header -> uid, uidBuff);

    // get gname and save to header
    char gidBuff[32];
    struct group *gr = getgrgid(fileStat.st_gid);
    sprintf(gidBuff, "%ld", (long)gr->gr_gid);
    strcpy(header -> gname, gr -> gr_name);

    free(buf);

    // get file type and save to header
    if(S_ISDIR(fileStat.st_mode) != 0){
        // directory
        header -> typeflag = DIRTYPE;
    } else if(S_ISLNK(fileStat.st_mode) != 0) {
        // link so we dont put a size just leave as zeros
        header -> typeflag = SYMTYPE;
        strcpy( header -> size,  "00000000000");
        readlink(filename, header -> linkname, 100);
    } else {
        // not a dir or link
        header -> typeflag = REGTYPE;
    }


    // TODO: compute checksum here and put in header
    unsigned int checksum = 0;
    memset(header -> checksum, ' ', sizeof(char) * 8);
    int l;
    char sum[512];
    memcpy(sum, header, 512);
    for(l = 0; l < 500; l++){
        checksum += sum[l];
    }
    char checkBuff[12];
    sprintf(checkBuff, "%06o", checksum);
    strcpy(header -> checksum, checkBuff);

    return 0;
}
