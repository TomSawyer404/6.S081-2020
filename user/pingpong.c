/* pingpong.c
 * @author      MrBanana
 * @version     0.1
 * @date        2020/11/17
 * @license     The MIT License
 * Updated on 2021-5-4
 **/

#include "user/user.h"

int main(int argc, char* argv[]) {
    if(argc > 2) {
        fprintf(2, "Too much arguments!\n");
        exit(1);
    }

    /* Create a pipe */
    int my_pipe[2];
    if( -1 == pipe(my_pipe) ) {
        fprintf(2, "pipe() failed!\n");
        exit(1);
    }

    /* Fork and play pingpong */
    char buf[8] = "ABCD";
    switch( fork() ) {
        case -1: 
        {
            fprintf(2, "fork(): error!");
            exit(1);
            break;
        }
        
        /* Child read from pipe */
        case 0:
        {
            while(1) {
                if( 0x1 == read(my_pipe[0], buf, 0x1) ) {
                    fprintf(1, "%d: received ping\n", getpid());

                    close( my_pipe[0] );    // close unused read-end
                    if( 0x1 != write(my_pipe[1], buf, 0x1) ) {
                        fprintf(2, "write(): failed in child!\n");
                        close( my_pipe[1] );
                        exit(1);
                    }
                    close( my_pipe[1] );    // close unused write-end
                    exit(0);
                }
            }
            break;
        }

        /* Parent read from pipe */
        default:
        {
            if( 0x1 != write(my_pipe[1], buf, 0x1) ) {
                fprintf(2, "write(): failed in parent!\n");
                exit(1);
            }

            close( my_pipe[1] );            // close unused write-end
            wait(0x0);
            while(1) {
                if( 0x1 == read(my_pipe[0], buf, 0x1) ) {
                    fprintf(1, "%d: received pong\n", getpid());
                    close( my_pipe[0] );    // close unused read-end
                    exit(0);
                }
            }
            break;
        }
    }

    return 0;
}
