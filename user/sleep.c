/* sleep.c
 * - Use sleep() system call to sleep a while.
 * @author      MrBanana
 * @version     0.1
 * @date        2020/11/16
 * @license     The MIT License
 * 
 * Updated on 2021-5-4
 **/

#include "user/user.h"

int main(int argc, char* argv[]) {
    if(argc != 2) {
        fprintf(2, "Usage: %s [integer]\n", argv[0]);
        exit(1);
    }
    if( sleep( atoi(argv[1]) ) == -1 ) {
        fprintf(2, "sleep() failed!\n");
        exit(1);
    }
    
    return 0;
}
