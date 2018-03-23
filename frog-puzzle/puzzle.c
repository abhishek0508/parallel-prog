/*
 * @author: João Gabriel Basi Nº USP: 9793801
 * @author: Juliano Garcia de Oliveira Nº USP: 9277086
 *
 * MAC0219
 * 26/03/19
 *
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "error.h"
#include "macros.h"

#define VAR 100

pthread_mutex_t *board_mtx, counter_mtx;
pthread_barrier_t bar;
pthread_t *frogs;
int *board;
int counter, N;

struct frog_t {
    int pos, id;
};


void debugPond(){
    printf("|");
    for (int i = 0; i < N - 1; i++)
        if (board[i] > 0)
            printf("\x1b[96m%2d\x1b[0m|", board[i]);
        else if (board[i] < 0)
            printf("\x1b[95m%2d\x1b[0m|", board[i]);
        else
            printf("%2d|", board[i]);
    if (board[N - 1] > 0)
        printf("\x1b[96m%2d\x1b[0m|\n", board[N - 1]);
    else if (board[N - 1] < 0)
        printf("\x1b[95m%2d\x1b[0m|\n", board[N - 1]);
    else
        printf("%2d|\n", board[N - 1]);


}
int sign(int x) {
    return (x < 0)? -1 : 1;
}


void *frog(void *args) {
    struct frog_t *f = (struct frog_t*)args;
    int pos = f->pos;
    int id = f->id;
    int type = sign(f->id);
    int next, lastPos, oldPos;
    pthread_barrier_wait(&bar);
    while (counter < VAR*N) {
        next = pos + type;
        lastPos = pos;
        if (next < N && next >= 0 && !board[next]) {
            printf("Frog %d is waiting to jump to %d\n", id, next);
            P(board_mtx + next);
            if (!board[next]) {
                printf("%d: My destiny is free!\n", id);
                P(board_mtx + pos);
                oldPos = pos;
                printf("Frog %d jumped from %d to %d\n", id, pos, next);
                board[pos] = 0;
                board[next] = id;
                pos = next;
                V(board_mtx+oldPos);
                printf("ESPERANDO MUTEX counterNormal.... %d\n", id);
                P(&counter_mtx);
                counter = 0;
                V(&counter_mtx);
            }
            V(board_mtx + next);
        }
        if (lastPos == pos && (next += type) < N && next >= 0 && !board[next]) {
            printf("Frog %d is waiting to jump to %d\n", id, next);
            P(board_mtx + next);
            if (!board[next]) {
                printf("%d: My destiny is free!\n", id);
                P(board_mtx + pos);
                oldPos = pos;
                printf("Frog %d jumped from %d to %d\n", id, pos, next);
                board[pos] = 0;
                board[next] = id;
                pos = next;
                V(board_mtx + oldPos);
                printf("ESPERANDO MUTEX (counter).... %d\n", id);
                P(&counter_mtx);
                counter = 0;
                V(&counter_mtx);
            }
            V(board_mtx + next);
        }
        if (lastPos == pos) {
            printf("ESPERANDO MUTEX(counter lastPos).... %d\n", id);
            P(&counter_mtx);
            counter++;
            printf("Frog %d failed...\n", id);
            V(&counter_mtx);
        }
    }
    printf("Frog %d quited\n", id);
    free(args);
    debugPond();
    printf("ESPERANDO BARREIRA.... %d\n", id);
    pthread_barrier_wait(&bar);
    pthread_exit(NULL);
}

struct frog_t *createFrog(int pos, int id) {
    struct frog_t *f = emalloc(sizeof(struct frog_t));
    f->pos = pos;
    f->id = id;
    return f;
}


int main(int argc, char const *argv[]) {
    set_prog_name("frog-puzzle");
    // frogs
    N = 3;
    int mFrogs = (N - 1)/2;
    int fFrogs = (N - 1)/2;
    struct frog_t *f;
    // number of stones in total

    // initializing structs
    counter = 0;
    board = emalloc(N*sizeof(int));
    board_mtx = emalloc(N*sizeof(pthread_mutex_t));
    frogs = emalloc((N - 1)*sizeof(pthread_t));
    pthread_barrier_init(&bar, NULL, N);
    pthread_mutex_init(&counter_mtx, NULL);
    for (int i = 0; i < N; i++)
        pthread_mutex_init(board_mtx + i, NULL);
    for (int i = 0; i < mFrogs; i++){
        f = createFrog(i, i + 1);
        pthread_create(frogs + i, NULL, &frog, (void*)f);
        board[i] = i + 1;
    }
    for (int i = 0; i < fFrogs; i++) {
        f = createFrog(N - i - 1, -(i + 1));
        pthread_create(frogs + N - i - 1, NULL, &frog, (void*)f);
        board[N - i - 1] = -(i + 1);
    }
    board[mFrogs] = 0;
    printf("I'M WAITING\n\n\n\n\n");
    pthread_barrier_wait(&bar);
    pthread_barrier_wait(&bar);

    for (int i = 0; i < N; i++)
        printf("%d, ", board[i]);
    printf("\n");

    free(board);
    free(board_mtx);
    free(frogs);
    pthread_exit(NULL);
}