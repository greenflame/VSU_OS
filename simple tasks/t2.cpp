#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
	int a = 10;

	for (int i = 0; i < 7; i++)
	{
		pid_t fr = fork();
	
		if (fr == 0)
		{
			printf("Child : %d\n", a);

			while(1)
			{
				sleep(10);
			}

			return 0;
		}
	}

	printf("Parent : %d\n", a);

	while(1)
	{
		sleep(10);
	}

	return 0;
}
