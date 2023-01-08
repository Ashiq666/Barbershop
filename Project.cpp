
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <stdlib.h>
#include <unistd.h>
using namespace std;

sem_t sofaEmpty, barberChairEmpty, customerLimit;
sem_t sofaFull, barberChairFull;
sem_t cashRegister;
pthread_mutex_t lockSofa, lockBarberChair;

queue<int> sofaBuffer;
queue<int> barberBuffer;

void init_semaphore()
{
    sem_init(&sofaEmpty, 0, 4);
    sem_init(&sofaFull, 0, 0);
    pthread_mutex_init(&lockSofa, 0);

    sem_init(&barberChairEmpty, 0, 3);
    sem_init(&barberChairFull, 0, 0);
    pthread_mutex_init(&lockBarberChair, 0);

    sem_init(&cashRegister, 0, 1);

    sem_init(&customerLimit, 0, 20);
}

void *Customer(void *arg)
{
    int customerID = *(int *)arg;
    printf("I am customer %d\n", customerID);

    sem_wait(&sofaEmpty);
    pthread_mutex_lock(&lockSofa);
    sleep(1);

    sofaBuffer.push(customerID);
    printf("Customer %d sit in sofa\n", customerID);

    pthread_mutex_unlock(&lockSofa);
    sem_post(&sofaFull);

    free(arg);
}

void *Barber(void *arg)
{
    int barberID = *(int *)arg;
    printf("I am Barber %d\n", barberID);

    while (true)
    {
        sem_wait(&sofaFull);
        pthread_mutex_lock(&lockSofa);
        sleep(1);
        int cus = sofaBuffer.front();
        sofaBuffer.pop();
        printf("Customer %d left the sofa for haircut\n", cus);

        pthread_mutex_unlock(&lockSofa);
        sem_post(&sofaEmpty);

        sem_wait(&barberChairEmpty);
        pthread_mutex_lock(&lockBarberChair);
        sleep(1);
        barberBuffer.push(cus);
        printf("Customer %d sit in barber's chair \n", cus);

        pthread_mutex_unlock(&lockBarberChair);
        sem_post(&barberChairFull);

        sem_wait(&barberChairFull);
        pthread_mutex_lock(&lockBarberChair);

        int i = barberBuffer.front();
        sleep(2);

        sem_wait(&cashRegister);
        printf("Customer %d has done the payment \n", i);
        sem_post(&cashRegister);

        barberBuffer.pop();

        pthread_mutex_unlock(&lockBarberChair);
        sem_post(&barberChairEmpty);

        printf("Customer %d left the shop \n", i);
        sem_post(&customerLimit);
    }
    free(arg);
}

int main(void)
{
    pthread_t thread2[3];

    init_semaphore();

    for (int i = 0; i < 3; i++)
    {
        int *id = (int *)malloc(sizeof(int));
        *id = i + 1;
        if (pthread_create(&thread2[i], NULL, &Barber, id))
        {
            perror("Could not create a thread\n");
            return 0;
        }
    }
    int i = 1;

    while (1)
    {
        sem_wait(&customerLimit);
        pthread_t customerThread;
        int *id = (int *)malloc(sizeof(int));
        *id = i;
        if (pthread_create(&customerThread, NULL, &Customer, id))
        {

            perror("Could not create a thread\n");
            return 0;
        }
        i++;
    }

    while (1);

    return 0;
}
