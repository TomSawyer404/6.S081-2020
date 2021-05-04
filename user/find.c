/* find.c
 * @author      MrBanana
 * @version     0.1
 * @date        2021/2/10
 * @license     The MIT License
 * @note:       Look at user/ls.c to see how to
 *              operate files and directories.
 **/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

void
do_find(char* path, char* filename)
{
    int fd = open(path, 0);
    if(fd < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    
    struct stat fileStat_st;
    if(fstat(fd, &fileStat_st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        return;
    }

    struct dirent dirent_st;
    while( read(fd, &dirent_st, sizeof(dirent_st)) == sizeof(dirent_st) ) {
        char buf[512] = {0x0};
        strcpy(buf, path);
        char* p = buf + strlen(buf);
        *p++ = '/';     // Append a '/' to suffix of path
        
        if( 0 == dirent_st.inum ) {
            continue;
        } 
        memmove(p, dirent_st.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if( stat(buf, &fileStat_st) < 0 ) {
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }
        
        switch(fileStat_st.type) {
            case T_FILE: 
                {
                    if( 0 == strcmp(filename, dirent_st.name) )
                        printf("%s\n", buf);
                    break;
                }
            case T_DIR:
                {
                    // DFS to next directory except '.' and '..'
                    if( strcmp(dirent_st.name, ".") != 0 && 
                            strcmp(dirent_st.name, "..") != 0 ) 
                        do_find(buf, filename);
                    break;
                }
        }
    }
    close(fd);
}

int 
main(int argc, char* argv[]) 
{
    if(argc != 3) {
        fprintf(2, "usage: find [dir] [file]\n");
        exit(1);
    }
    do_find(argv[1], argv[2]);

    return 0;
}
