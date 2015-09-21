#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

bool stop = false;

void stopChild(int signum)
{
	stop = true;
}

// First param - text file, second - word to search
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "sth wrong with args" << endl;
		return 0;
	}


	int fd[2];
	pipe(fd);	// Creating pipe

	pid_t childPid = fork();

	if (childPid)	// Parent process
	{
		close(fd[0]);	//Close pipe for read

		// Reading file
		ifstream fin(argv[1]);	// Open stream
		string line;
		while(getline(fin, line))	// Next line
		{
			line += '\n';	// Add eol symbol
			write(fd[1], line.c_str(), strlen(line.c_str()) + 1);	// Writing line for pipe
		}

		kill(childPid, SIGUSR1); // Send to child stop signal

		cout << "parent stop" << endl;
		return 0;
	}
	else	// Child process
	{
		close(fd[1]); // Close pipe for write

		signal(SIGUSR1, stopChild); // Connecting to signal

		char readBuffer[1];	// Char buffer
		string line = "";	// String buffer
		bool previousReadSucces = false;

		while (!stop || previousReadSucces)	// While parent works or buffer not empty
		{
			int readed = read(fd[0], readBuffer, 1); // Trying to read one symbol

			if (readed != 0)	// If succesfull
			{
				previousReadSucces = true;

				if (readBuffer[0] == '\n') // If symbol is eol
				{
					if (line.find(string(argv[2])) != -1)
					cout << line << endl;	// Output line
					line = "";	// Clear line
				}
				else
				{
					line += readBuffer[0];	// Else add symbol to line
				}
			}
			else
			{
				previousReadSucces = false;
			}
		}

		cout << "child stop" << endl;
		return 0;
	}
}
