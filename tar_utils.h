/*
 * tar_utils.h
 * Author: Kevin Littell
 * Date: 10-19-2018
 * COSC 3750, program 5
 * replacment for the tar utility
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <tar.h>
//#include <dirent.h>

#define REGTYPE '0'                   // regular file
#define AREGTYPE '\0'                 // regular file
#define SYMTYPE '2'                   // symbolic link
#define DIRTYPE '5'


struct t_header {               // byte offset //
    char name[100];             // 0
    char mode[8];               // 100
    char uid[8];                // 108
    char gid[8];                // 116
    char size[12];              // 124
    char mtime[12];             // 136
    char checksum[8];           // 148
    char typeflag;              // 156
    char linkname[100];         // 157
    char magic[6];              // 257
    char version[2];            // 263
    char uname[32];             // 265
    char gname[32];             // 297
    char devmajor[8];           // 329
    char devminor[8];           // 337
    char prefix[155];           // 345
    char pad[12];               // 500
};

int tArchive(FILE * fd, struct t_header ** arc,
    const int fc, const char * files[]);


int tWrite(FILE * fd, struct t_header ** myTar, struct t_header ** arc,
    const int fc, const char * files[]);

int tExtract(FILE * fd, struct t_header ** arc,
    const int fc, const char * files[], const char * tarName);

int tWriteEnd(FILE * fd);

int tStatFile(struct t_header * header, const char * filename);
