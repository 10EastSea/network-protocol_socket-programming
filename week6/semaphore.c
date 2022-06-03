#include <stdio.h>
#include <pthread.h>
// #include <semaphore.h> // 맥에서는 제약이 있어 제대로 동작 안함
#include <mach/mach_init.h>
#include <mach/semaphore.h>
#include <mach/task.h>


void* read(void* arg);
void* accu(void* arg);

static semaphore_t sem_one; // static sem_t sem_one;
static semaphore_t sem_two; // static sem_t sem_two;

static int num;

// 컴파일 명령어: gcc semaphore.c -D_REENTRANT -o sem -lpthread
int main(int argc, char* argv[]) {
    pthread_t id_t1, id_t2;

    mach_port_t self = mach_task_self(); // 맥 환경에서 필요에 의해 세팅
    semaphore_create(self, &sem_one, SYNC_POLICY_FIFO, 0); // sem_open(&sem_one, 0, 0);
    semaphore_create(self, &sem_two, SYNC_POLICY_FIFO, 1); // sem_open(&sem_two, 0, 1);

    pthread_create(&id_t1, NULL, read, NULL);
    pthread_create(&id_t2, NULL, accu, NULL);

    pthread_join(id_t1, NULL);
    pthread_join(id_t2, NULL);

    // sem_destroy(&sem_one);
    // sem_destroy(&sem_two);

    return 0;
}

void* read(void* arg) {
    int i;
    for(i=0; i<5; i++) {
        fputs("input num:", stdout);
        semaphore_wait(sem_two); // sem_wait(&sem_two);
        scanf("%d", &num);
        semaphore_signal(sem_one); // sem_post(&sem_one);
    }
    return NULL;
}

void* accu(void* arg) {
    int sum = 0, i;
    for(i=0; i<5; i++) {
        semaphore_wait(sem_one); // sem_wait(&sem_one);
        sum += num;
        semaphore_signal(sem_two); // sem_post(&sem_two);
    }
    printf("result: %d\n", sum);
    return NULL;
}