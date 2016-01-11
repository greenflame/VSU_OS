#include <pthread.h>

#include <iostream>
#include <fstream>

#include <queue>
#include <string>

std::queue<std::string> q;

pthread_mutex_t lock;

void *thr_f(void *arg)
{
	std::string toFind(((char**)arg)[2]);

	while (true)
	{
		std::string line;
		bool isEmpty = true;

		pthread_mutex_lock(&lock);

		if (!q.empty())
		{
			line = q.front();
			q.pop();

			isEmpty = false;
		}

		pthread_mutex_unlock(&lock);

		if (!isEmpty)
		{
			if (line != "end of text")
			{
				if (line.find(toFind) != -1)
				{
					std::cout << line << std::endl;
				}
			}
			else
			{
				return NULL;
			}
		}

	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "sth wrong" << std::endl;
		return 0;
	}

	pthread_mutex_init(&lock, NULL);

	pthread_t thr_id;
	pthread_create(&thr_id, NULL, thr_f, (void*)argv);

	std::ifstream fin(argv[1]);
	std::string line;
	while (getline(fin, line))
	{
		pthread_mutex_lock(&lock);
		q.push(line);
		pthread_mutex_unlock(&lock);
	}

	pthread_mutex_lock(&lock);
	q.push("end of text");
	pthread_mutex_unlock(&lock);

	pthread_join(thr_id, NULL);
	
	pthread_mutex_destroy(&lock);

	return 0;
}