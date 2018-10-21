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
        if (strcmp(argv[1], "-c") == 0 || strcmp(argv[2], "-c") == 0 ||
            strcmp(argv[3], "-c") == 0){
            create = true;
            pos++;
        }
        if (strcmp(argv[1], "-x") == 0 || strcmp(argv[2], "-x") == 0 ||
            strcmp(argv[3], "-x") == 0 ){
            extract = true;
            pos++;
        }
        if (strcmp(argv[1], "-f") == 0 || strcmp(argv[2], "-f") == 0 ||
            strcmp(argv[3], "-f") == 0 ){
            pos++;
            arcName = argv[pos];
        } else {
            printf("Error: no archive name specified\n");
        }
    }
    return 0;
}
