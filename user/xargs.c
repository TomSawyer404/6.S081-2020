/* xargs.c
 * @author:         MrBanana
 * @version:        0.1
 * @date:           2021/02/15
 * @license:        The MIT License
 * @note:           A simple version of the UNIX xargs
 **/

#include "kernel/types.h"
#include "kernel/param.h"
#include "user.h"

#define MAX_STDIN       512

void 
fmt_args(char* read_in, int len, char** args, int* args_cnt) 
{
    /* Convet input into a argv list */
    char cur_buf[MAX_STDIN] = {0x0};
    int cur_buf_len = 0;
    for(int i = 0; i <= len; i++) {
        if( (read_in[i] == ' ' || read_in[i] == '\n') && cur_buf_len ) {
            // read a new arg
            args[*args_cnt] = malloc(cur_buf_len + 1);
            memcpy(args[*args_cnt], cur_buf, cur_buf_len);
            args[*args_cnt][cur_buf_len] = 0x0;
            cur_buf_len = 0;
            (*args_cnt)++;
        }
        else {
            cur_buf[cur_buf_len] = read_in[i];
            cur_buf_len++;
        }
    }
}

int
main(int argc, char* argv[]) 
{
    // Read args behind xargs
    char* args[MAXARG + 1] = {0x0};
    int args_cnt = 0;
    for(int i = 1; i < argc; i++, args_cnt++) {
        args[args_cnt] = malloc(sizeof(argv[i]));
        memcpy(args[args_cnt], argv[i], sizeof(argv[i]));
    }

    // Read args from stdin
    int result = -1;
    char stdin_buf[MAX_STDIN] = {0x0};
    if( (result = read(0, stdin_buf, sizeof(stdin_buf))) > 0 ) {
        fmt_args(stdin_buf, strlen(stdin_buf), args, &args_cnt);
    }

    exec(args[0], args);
    return 0;
}
