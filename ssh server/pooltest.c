#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define THREADS 2

pthread_t			threads[THREADS];
int					tasks[THREADS];
pthread_mutex_t		locks[THREADS];

void *thr_f(void *arg)
{
	int id = (int)arg;

	while (1)
	{		
		printf("Thread %d waiting for task\n", id);			

		pthread_mutex_lock(&locks[id]);
		pthread_mutex_lock(&locks[id]);
		pthread_mutex_unlock(&locks[id]);
		
		printf("Thread %d start task\n", id);
		
		// Do task
		sleep(2);
		printf("%d\n", tasks[id]);
		sleep(2);
	}	
	
	return 0;
}

void pool_init()
{
	for (int i = 0; i < THREADS; i++)
	{
		pthread_mutex_init(&locks[i], NULL);
		pthread_create(&threads[i], NULL, thr_f, (void *)(long long)i);
	}
}

void pool_destroy()
{
	for (int i = 0; i < THREADS; i++)
	{
		pthread_kill(threads[i], 0);
		pthread_mutex_destroy(&locks[i]);
	}
}

void pool_give_task(int task)
{
	for (int i = 0; i < THREADS; i++)
	{
		if (pthread_mutex_trylock(&locks[i]))	// Is close, cann't lock
		{
			tasks[i] = task;
			pthread_mutex_unlock(&locks[i]);
			return;	// Stop search
		}
		else	// If was open, open again
		{
			pthread_mutex_unlock(&locks[i]);
		}
	}
	
	printf("[info]: All threads are bisy. Task declined.\n");
}

int main()
{
	pool_init();
	sleep(1);

	pool_give_task(4);
	sleep(1);
	pool_give_task(5);
	sleep(1);
	pool_give_task(6);
	sleep(1);
	pool_give_task(7);
	sleep(1);
	pool_give_task(8);
	sleep(1);


	sleep(20);
	pool_destroy();
	
	return 0;
}