/*
 * wytar.c
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
 #include <fcntl.h>
 #include <dirent.h>
 #include <pwd.h>
 #include <grp.h>
 #include <time.h>
 #include <sys/time.h>
 #include <stdbool.h>
 #include <math.h>

#include "tar.h"
#include "tar_utils.h"

int main (int argc, char * argv[]){

    bool create = false;
    bool extract = false;

    int pos = 1;

    char * arcName;

    // check options
    if(argc >= 3){
        if (strcmp(argv[1], "-c") == 0){
            create = true;
            pos++;
            if (strcmp(argv[1], "-f") == 0 || strcmp(argv[2], "-f") == 0 ||
                strcmp(argv[3], "-f") == 0 ){
                pos++;
                arcName = argv[pos];
                pos++;
            } else {
                printf("Error: no archive name specified\n");
            }
        }else if (strcmp(argv[1], "-x") == 0){
            extract = true;
            pos++;
            if (strcmp(argv[1], "-f") == 0 || strcmp(argv[2], "-f") == 0 ||
                strcmp(argv[3], "-f") == 0 ){
                pos++;
                arcName = argv[pos];
                pos++;
            } else {
                printf("Error: no archive name specified\n");
            }
        } else {
            printf("Options werent specified correctly\n");
            printf("-c or -x, then -f followed by the name of the files\n");
            printf("to archive or extract");
        }


    }
    if (create && extract){
        printf("Error: unable to extract and archive at once\n");
    }
    if (!create && !extract){
        printf("Error: must specify wheather to archive or extract\n");
    }

    const char ** files = (const char **) &argv[4];
    FILE * fd;
    if (create){
        fd = fopen(arcName, "wb");
        if (fd == NULL){
            printf("fopen failed, errno = %d\n", errno);
            return -1;
        }
        struct t_header * arc = NULL;
        if(tArchive(fd, &arc, (argc - pos),files) < 0){
            fprintf(stderr, "Error: Failed to write archive\n");
        }
    } else {
        fd = fopen(arcName, "rb");
        if (fd == NULL){
            printf("fopen failed, errno = %d\n", errno);
            return -1;
        }
        struct t_header * arc = NULL;
        tExtract(fd, &arc, (argc - pos),files, argv[3]);
        //     fprintf(stderr, "Error: Failed to open archive\n");
        // }
        //if(t)
    }
    return 0;
}
