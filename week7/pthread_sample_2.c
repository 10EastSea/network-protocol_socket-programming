#include <stdio.h>
#include <pthread.h>

void* thread_summation(void* arg);

int sum = 0;

// 컴파일 명령어: gcc pthread_sample_2.c -D_REENTRANT -o th2 -lpthread
int main(int argc, char* argv[]) {
    pthread_t id_t1, id_t2;
    int range1[] = {1, 5};
    int range2[] = {6, 10};

    pthread_create(&id_t1, NULL, thread_summation, (void*)range1);
    pthread_create(&id_t2, NULL, thread_summation, (void*)range2);

    pthread_join(id_t1, NULL); // 결과값 받아오지 않음
    pthread_join(id_t2, NULL);

    printf("result: %d\n", sum);
    return 0;
}

void* thread_summation(void* arg) {
    int start = ((int*)arg)[0];
    int end = ((int*)arg)[1];

    while(start <= end) {
        sum += start;
        start++;
    }

    return NULL; // 세마포어를 쓰지 않아서 경우에 따라 55가 안나올 수 있음..
}