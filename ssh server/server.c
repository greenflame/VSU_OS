#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080
#define BACKLOG 10
#define THREADS 2

pthread_t			threads[THREADS];
int					tasks[THREADS];
pthread_mutex_t		locks[THREADS];

void execute_bash(int cd, char *cmd)
{	
	FILE *fp = popen(cmd, "r");
	if (fp == NULL) {
		char *msg = "Failed to run command\r\n\0";
		send(cd, msg, strlen(msg), 0);
	}
	
	// Reading output, writing to socket
	char buf[1000];
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		send(cd, buf, strlen(buf), 0);
	}
	
	pclose(fp);
}

void connection_processor(int cd)
{
		printf("[net.info]: Socket acepted [%d]\n", cd);
				
		while (1)
		{
			// Command prefix for client
			char *msg = "User@ssh_server$ \0";
			send(cd, msg, strlen(msg), 0);			
			
			// Accept command
			char cmd[1000];
			memset(cmd, '\0', sizeof(cmd));
			recv(cd, &cmd, sizeof(cmd), 0);
			cmd[strlen(cmd) - 2] = '\0';	// Delete \r\n

			// Printing command on server
			printf("[net.command]: %s\n", cmd);

			// Switch command
			if (strcmp(cmd, "hello\0") == 0)
			{
				// Greeting command
				char *msg = "Hello, user!\r\n\0";
				send(cd, msg, strlen(msg), 0);
			}
			else if (strcmp(cmd, "exit\0") == 0)
			{
				// Close connection command
				char *msg = "Good bye!\r\n\0";
				send(cd, msg, strlen(msg), 0);
				
				close(cd);
				break;
			}
			else
			{
				// Execute command (default)
				execute_bash(cd, cmd);
			}	
		}
		
		printf("[net.info]: Socket closed [%d]\n", cd);
}

void *thread_function(void *arg)
{
	int id = (int)arg;

	while (1)
	{		
		printf("[tp.info]: Thread %d waiting for task\n", id);			

		pthread_mutex_lock(&locks[id]);
		pthread_mutex_lock(&locks[id]);
		pthread_mutex_unlock(&locks[id]);
		
		printf("[tp.info]: Thread %d start task\n", id);
		
		// Do task
		connection_processor(tasks[id]);
	}	
	
	return 0;
}

void pool_init()
{
	for (int i = 0; i < THREADS; i++)
	{
		pthread_mutex_init(&locks[i], NULL);
		pthread_create(&threads[i], NULL, thread_function, (void *)(long long)i);
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
	
	close(task);	// Close connection
	printf("[tp.info]: All threads are bisy. Task declined.\n");
}

void start_server()
{
	int ld = socket(AF_INET, SOCK_STREAM, 0);
	if (ld == -1)
	{
		printf("[error]: Listener create error\n");
	}

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8080);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	
	socklen_t size_saddr;
	
	int bindRes = bind(ld, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (bindRes == -1)
	{
		printf("[error]: Bind error\n");
	}
	
	int listenRes = listen(ld, BACKLOG);
	if (listenRes == -1)
	{
		printf("[error]: Listen error\n");
	}
	
	while (1)
	{
		struct sockaddr_in client_addr;
		socklen_t size_caddr;
		
		int cd = accept(ld, (struct sockaddr *)&client_addr, &size_caddr);
		if (cd == -1)
		{
			printf("[error]: Accept error\n");
		}
		
		pool_give_task(cd);
	}
}

int main()
{
	pool_init();
	start_server();
	pool_destroy();
	return 0;
}
