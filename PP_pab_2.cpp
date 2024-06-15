// variant 7. functions 2, 5, 11. a = -pi, b = pi, n = 8

#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <iomanip>
#include <iostream>

#define M_PI 3.14159265358979323846

struct arguments{
    float a;
    float b;
    int n;
};

pthread_barrier_t barrier;

double res1, res2, res3;

void * func1(void * arguments){
    struct arguments * args = (struct arguments*) arguments; // 
    
    float step = (args -> b - args -> a) / args -> n;
    float argument = args -> a;

    for (int i = 0; i < args -> n; i++){
        usleep(1);

        argument = args -> a + i * step;
        res1 = pow(cos(argument), 2) + sin(argument);

        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

void * func2(void * arguments){
    struct arguments * args = (struct arguments*) arguments;
    
    float step = (args -> b - args -> a) / args -> n;
    float argument = args -> a;

    for (int i = 0; i < args -> n; i++){
        usleep(1);

        argument = args -> a + i * step;
        res2 = pow(sin(argument), 2) * (1 + cos(argument));

        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

void * func3(void * arguments){
    struct arguments * args = (struct arguments*) arguments;
    
    float step = (args -> b - args -> a) / args -> n;
    float argument = args -> a;

    for (int i = 0; i < args -> n; i++){
        usleep(1);

        argument = args -> a + i * step;
        res3 = (1 + pow(cos(argument), 2)) * sin(argument);

        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

int main(){

    struct arguments function_args;

    function_args.a = -M_PI;
    function_args.b = M_PI;
    function_args.n = 8;

    pthread_t func1_thread, func2_thread, func3_thread;

    pthread_barrier_init(&barrier, NULL, 4);

    pthread_create(&func1_thread, NULL, &func1, &function_args); // creating and starting threads
    pthread_create(&func2_thread, NULL, &func2, &function_args); // passing arguments
    pthread_create(&func3_thread, NULL, &func3, &function_args);

    // printf("\tx\t|  cos(x)^2 + sin(x)  |  sin(x)^2 * (1 + cos(x))\n");

    std::cout << std::setw(10) << "x" << std::setw(24) << "cos(x)^2+sin(x)" << std::setw(24) << "sin(x)^2*(1+cos(x))" << std::setw(24) <<  "sin(x)*(1+cos(x)^2)" << std::endl;

    float step = (function_args.b - function_args.a) / function_args.n;
    float argument = function_args.a;

    for (int i = 0; i < function_args.n; i++){

        pthread_barrier_wait(&barrier);
        argument += step;
        std::cout << std::setw(10) << argument << std::setw(24) << res1 << std::setw(24) << res2 << std::setw(24) << res3 << std::endl;
    }

    pthread_join(func1_thread, NULL); // waiting threads to finish
    pthread_join(func2_thread, NULL);
    pthread_join(func3_thread, NULL);

    return 0;
}