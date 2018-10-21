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

int tArchive(const int fd, struct t_header ** arc,
    const int fc, const char * files[]);


int tWrite(const int fd, struct t_header ** arc,
    const int fc, const char * files[]);

int tWriteEnd(const int fd, int offset);

int tStatFile(struct t_header * header, const char * filename);
