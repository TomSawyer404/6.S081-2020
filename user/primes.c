/* primes.c
 * @author      MrBanana
 * @version     0.2
 * @date        2020/11/28
 * @license     The MIT License 
 * @note:   This is the version 2 of primes, using
 *          strict standard descripted by assignment.
 **/

#include "user/user.h"

int COUNTER = 0;    // Record numbers of integers we've written into pipe.

void sievePrimer(int, int*, int*);

int main(int argc, char* argv[]) {
    if(argc > 1) {
        fprintf(2, "Too much arguments\n");
        exit(1);
    }

    /* Define an array:
     *   If value eq 0x1, then it was printed
     *   If value eq 0x2, then it was droped
     **/
    int array[36] = {0};
    int myPipe[2] = {0};
    int primeNum = 2;
    pipe(myPipe);
    for(int i = primeNum; i <= 35; i++) {       // Feed pipe
        if(array[i] == 0x0) {
            write(myPipe[1], &i, sizeof(int));
            COUNTER++;
        }
    }
    sievePrimer(primeNum, array, myPipe);

    exit(0);
}

void sievePrimer(int primeNum, int* array, int* myPipe) {
/* @brief:
 *   Fork a child and sieve array using 
 *   Eratosthenes's method
 **/
    int pid = -1;
loop:
    pid = fork();
    if(pid < 0) {
        fprintf(2, "fork(): error!\n");
        exit(1);
    } else if(pid > 0) {
        wait(0x0);
    } else {
        /* Read from pipe */
        int buf[36] = {0x0};
        int tmp = -1;
        for(int i = 0; i < COUNTER; i++) {
            read(myPipe[0], &tmp, sizeof(int));
            if(tmp == primeNum) {
                printf("prime %d\n", tmp);
                array[tmp] = 0x1;       // mark as printed
            } else if(tmp % primeNum == 0) {
                array[tmp] = 0x2;       // mark as dropped
            } else {
                buf[tmp] = 0x3;         // ready to feed next
            }
        }
        COUNTER = 0x0;

        /* Feed left number into pipe */
        for(int i = primeNum; i <= 35; i++ ) {
            int tmp = i;
            if( buf[i] == 0x3 ) {
                write(myPipe[1], &tmp, sizeof(int));
                COUNTER++;
            }
        }
        
        /* Update primeNum and loop till the end */
        for(int i = primeNum+1; i <= 35; i++) { 
            if(array[i] != 0x1 && array[i] != 0x2) {
                primeNum = i;
                break;
            }
        }
        if(COUNTER != 0)
            goto loop;
    }
}
