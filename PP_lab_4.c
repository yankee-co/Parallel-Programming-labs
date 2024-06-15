// Variant 7 Zemlyanskyi Eduard KV-22
// scheme 4

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

#define STACK_LEN 15
#define EMPTY (-1)

int CR1[STACK_LEN];
int top = EMPTY;

int push(int value){
    if (top >= STACK_LEN - 1) return 0;
    top++;
    CR1[top] = value;
    return 1;
}

int pop(){
    if (top == EMPTY) return -1;
    int result = CR1[top];
    top--;
    return result;
}

FILE *log_file;

pthread_t P1;
pthread_t P2;
pthread_t P3;
pthread_t P4;
pthread_t P5;
pthread_t P6;

pthread_mutex_t MCR1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Signal_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Sig21 = PTHREAD_COND_INITIALIZER;
pthread_cond_t Sig22 = PTHREAD_COND_INITIALIZER;
sem_t SCR1;
pthread_barrier_t BCR2;

int flag_for_P1_1 = 0;
int flag_for_P1_2 = 0;
int flag_for_P2 = 0;
int flag_for_P4_1 = 0;
int flag_for_P4_2 = 0;

//Atomic variables
int i1 = 1;
int i2 = 2;
unsigned int ui1 = 3;
unsigned int ui2 = 4;
long int li1 = 5;
long int li2 = 6;
unsigned long int uli1 = 7;
unsigned long int uli2 = 8;

// Atomic operations 1 3 4 8 11 12 13 14
// 1. type __atomic_add_fetch (type *ptr, type val, int memorder)
// 3. type __atomic_and_fetch (type *ptr, type val, int memorder)
// 4. type __atomic_xor_fetch (type *ptr, type val, int memorder)
// 8. type __atomic_fetch_sub (type *ptr, type val, int memorder)
// 11. type __atomic_fetch_or (type *ptr, type val, int memorder)
// 12. type __atomic_fetch_nand (type *ptr, type val, int memorder)
// 13. bool __atomic_compare_exchange_n (type *ptr, type *expected, type desired, bool 
// weak, int success_memorder, int failure_memorder)
// 14. void __atomic_exchange (type *ptr, type *val, type *ret, int memorder)

void actions_for_P1(){
    fprintf(log_file, "P1 used atomic\n");
}
void actions_for_P2(){
    fprintf(log_file, "P2 used atomic\n");
}
void actions_for_P4(){
    fprintf(log_file, "P4 used atomic\n");
}

// CR2 (atomic variables) user
void* funcP1(void* unused){

    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while(1){

        if(top == EMPTY || top == STACK_LEN - 1) break;
        usleep(1);
        
        // sig21
        pthread_mutex_lock(&Signal_mutex);
        while(flag_for_P1_1 == 0){
            fprintf(log_file, "P1 waiting for Sig21\n");
            pthread_cond_wait(&Sig21,&Signal_mutex);
        }
        flag_for_P1_1 = 0;
        pthread_mutex_unlock(&Signal_mutex);

        //using atomic
        actions_for_P1();

        // full sync bcr2
        pthread_barrier_wait(&BCR2);

        // sig22
        pthread_mutex_lock(&Signal_mutex);
        while(flag_for_P1_2 == 0){
            fprintf(log_file, "P1 waiting for Sig22\n");
            pthread_cond_wait(&Sig22, &Signal_mutex);
        }
        flag_for_P1_2 = 0;
        pthread_mutex_unlock(&Signal_mutex);

        //using atomic
        actions_for_P1();
    }

    pthread_cancel(P2);
    pthread_cancel(P3);
    pthread_cancel(P4);
    pthread_cancel(P5);
    pthread_cancel(P6);

    fprintf(log_file, "P1 stopped all threads and program!\n");

    return NULL;
}

//CR1 and CR2 user
void* funcP2(void* unused){

    int sem_value;

    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while(1){
        if(top == EMPTY || top == STACK_LEN - 1) break;
        usleep(1);
        
        // sig21
        pthread_mutex_lock(&Signal_mutex);
        while(flag_for_P2 == 0){
            fprintf(log_file, "P2 waiting for Sig21\n");
            pthread_cond_wait(&Sig21, &Signal_mutex);
        }
        flag_for_P2 = 0;
        pthread_mutex_unlock(&Signal_mutex);

        //using CR2
        actions_for_P2();

        //using CR1
        sem_wait (&SCR1);
        pthread_mutex_lock(&MCR1);
        int taken = pop(); // taking out from stack
        if (taken == -1) break;
        sem_getvalue(&SCR1, &sem_value);
        fprintf(log_file, "P2 (Consumer) took %d from CR1[%d]. Semaphore value: %d\n", taken, top+1, sem_value);
        pthread_mutex_unlock(&MCR1);
    }

    pthread_cancel(P1);
    pthread_cancel(P3);
    pthread_cancel(P4);
    pthread_cancel(P5);
    pthread_cancel(P6);
    fprintf(log_file, "P2 stop all threads and program!\n");

    return NULL;
}

// CR1 user
void* funcP3(void* unused){

    int sem_value;
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while(1){
        if(top == EMPTY || top == STACK_LEN - 1) break;
        usleep(1);

        //using CR1
        sem_wait (&SCR1);
        pthread_mutex_lock(&MCR1);
        int taken = pop(); // taking out from stack
        if (taken == -1) break;
        sem_getvalue(&SCR1, &sem_value);
        fprintf(log_file, "P3 (Consumer) took %d from CR1[%d]. Semaphore value: %d\n", taken, top+1, sem_value);
        pthread_mutex_unlock(&MCR1);
    }

    pthread_cancel(P1);
    pthread_cancel(P2);
    pthread_cancel(P4);
    pthread_cancel(P5);
    pthread_cancel(P6);
    fprintf(log_file, "P3 stop all threads and program!\n");

    return NULL;
}

// CR2 user
void* funcP4(void* unused){

    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while(1){
        if(top == EMPTY || top == STACK_LEN - 1) break;
        usleep(1);

        // sig21
        pthread_mutex_lock(&Signal_mutex);
        while(flag_for_P4_1 == 0){
            fprintf(log_file, "P4 waiting for Sig21\n");
            pthread_cond_wait(&Sig21, &Signal_mutex);
        }
        flag_for_P4_1 = 0;
        pthread_mutex_unlock(&Signal_mutex);

        // sig22
        pthread_mutex_lock(&Signal_mutex);
        while(flag_for_P4_2 == 0){
            fprintf(log_file, "P4 waiting for Sig21\n");
            pthread_cond_wait(&Sig22, &Signal_mutex);
        }
        flag_for_P4_2 = 0;
        pthread_mutex_unlock(&Signal_mutex);

        //using and modifying CR2
        actions_for_P4();
        __atomic_add_fetch (&i1, 1, __ATOMIC_RELAXED);
        __atomic_and_fetch (&li1, 3, __ATOMIC_RELAXED);
        __atomic_xor_fetch (&li2, 7, __ATOMIC_RELAXED);
        __atomic_fetch_sub (&uli1, 5, __ATOMIC_RELAXED);
        __atomic_fetch_or (&uli2, 11, __ATOMIC_RELAXED);
        __atomic_fetch_nand (&ui1, 2, __ATOMIC_RELAXED);
        __atomic_exchange (&ui2, &ui1, &ui1, __ATOMIC_RELAXED);
        fprintf(log_file, "P4 modify all atomic\n");

        // full sync bcr2
        pthread_barrier_wait(&BCR2);

        //using and modifying CR2
        actions_for_P4();
        __atomic_add_fetch (&i1, 2, __ATOMIC_RELAXED);
        __atomic_and_fetch (&li1, 4, __ATOMIC_RELAXED);
        __atomic_xor_fetch (&li2, 8, __ATOMIC_RELAXED);
        __atomic_fetch_sub (&uli1, 6, __ATOMIC_RELAXED);
        __atomic_fetch_or (&uli2, 12, __ATOMIC_RELAXED);
        __atomic_fetch_nand (&ui1, 3, __ATOMIC_RELAXED);
        __atomic_exchange (&ui1, &ui2, &ui2, __ATOMIC_RELAXED);
        fprintf(log_file, "P4 modify all atomic\n");
    }

    pthread_cancel(P1);
    pthread_cancel(P2);
    pthread_cancel(P3);
    pthread_cancel(P5);
    pthread_cancel(P6);
    fprintf(log_file, "P4 stop all threads and program!\n");

    return NULL;
}

void* funcP5(void* unused){

    int sem_value;
    int num;
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while(1){
        if(top == EMPTY || top == STACK_LEN - 1) break;
        usleep(1);
        // modifying CR2 partly
        __atomic_xor_fetch (&li1, 6, __ATOMIC_RELAXED);

        // sending sig21
        pthread_mutex_lock(&Signal_mutex);
        fprintf(log_file, "P5 sent Sig21 for P1\n");
        fprintf(log_file, "P5 sent Sig21 for P4\n");
        flag_for_P4_1 = 1;
        flag_for_P1_1 = 1;
        pthread_cond_signal(&Sig21);
        pthread_mutex_unlock(&Signal_mutex);

        // modifying CR2 partly
        __atomic_and_fetch (&i2, 3, __ATOMIC_RELAXED);

        //sending sig22
        pthread_mutex_lock(&Signal_mutex);
        fprintf(log_file, "P5 sent Sig22 for P1\n");
        fprintf(log_file, "P5 sent Sig22 for P4\n");
        flag_for_P4_2 = 1;
        flag_for_P1_2 = 1;
        pthread_cond_signal(&Sig22);
        pthread_mutex_unlock(&Signal_mutex);

        // writing to CR1
        pthread_mutex_lock(&MCR1);
        num = rand()%10;
        if(push(num) == 0) break;
        sem_getvalue(&SCR1, &sem_value);
        fprintf(log_file,"P5 (Producer) write %d to CR1[%d]. Semaphore value: %d\n", num, top, sem_value);
        sem_post(&SCR1);
        pthread_mutex_unlock(&MCR1);
    }

    pthread_cancel(P1);
    pthread_cancel(P2);
    pthread_cancel(P3);
    pthread_cancel(P4);
    pthread_cancel(P6);
    fprintf(log_file, "P5 stop all threads and program!\n");

    return NULL;
}

void* funcP6(void* unused){

    int sem_value;
    int num;
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while(1){
        if(top == EMPTY || top == STACK_LEN - 1) break;
        usleep(1);
        
        // writing to CR1
        pthread_mutex_lock(&MCR1);
        num = rand()%10;
        if(push(num) == 0) break;
        sem_getvalue(&SCR1, &sem_value);
        fprintf(log_file,"P6 (Producer) write %d to CR1[%d]. Semaphore value: %d\n", num, top, sem_value);
        sem_post(&SCR1);
        pthread_mutex_unlock(&MCR1);
    }

    pthread_cancel(P1);
    pthread_cancel(P2);
    pthread_cancel(P3);
    pthread_cancel(P4);
    pthread_cancel(P5);

    fprintf(log_file, "P6 stop all threads and program!\n");

    return NULL;
}

int main(){

    sem_init (&SCR1, 0, 0);
    pthread_barrier_init(&BCR2, NULL, 2);

    log_file = fopen("logs.log", "w"); // opening log file

    fprintf(log_file, "\nAtomic variables:\ni1 = %d\ni2 = %d\nui1 = %u\nui2 = %u\nli1 = %ld\nli2 = %ld\nuli1 = %lu\nuli1 = %lu\n\n",
    i1,
    i2,
    ui1,
    ui2,
    li1,
    li2,
    uli1,
    uli2
    );

    // filling a bit of stack for good start
    for (int counter = 0; counter < 5; counter++){
        push(rand()%10); 
    }

    // creating and starting threads
    pthread_create (&P1,NULL,&funcP1,NULL); 
    pthread_create (&P2,NULL,&funcP2,NULL);
    pthread_create (&P3,NULL,&funcP3,NULL);
    pthread_create (&P4,NULL,&funcP4,NULL);
    pthread_create (&P5,NULL,&funcP5,NULL);
    pthread_create (&P6,NULL,&funcP6,NULL);

    pthread_join(P1,NULL); 
    pthread_join(P2,NULL);
    pthread_join(P3,NULL);
    pthread_join(P4,NULL);
    pthread_join(P5,NULL);
    pthread_join(P6,NULL);

    fprintf(log_file, "\nAtomic variables:\ni1 = %d\ni2 = %d\nui1 = %u\nui2 = %u\nli1 = %ld\nli2 = %ld\nuli1 = %lu\nuli1 = %lu\n",
    i1,
    i2,
    ui1,
    ui2,
    li1,
    li2,
    uli1,
    uli2
    );

    return 0;
}