#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include <iostream>
#include <fstream>
#include <string>

#define STOP_CHAR 10

using namespace std;

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

		write(fd[1], "~", 2);

		cout << "parent stop" << endl;
		return 0;
	}
	else	// Child process
	{
		close(fd[1]); // Close pipe for write

		char readBuffer[1];	// Char buffer
		string line = "";	// String buffer

		while (true)	// While parent works or buffer not empty
		{
			int readed = read(fd[0], readBuffer, 1); // Trying to read one symbol

			if (readed != 0)	// If succesfull
			{

				if (readBuffer[0] == '\n') // If symbol is eol
				{
					if (line.find(string(argv[2])) != -1)
					cout << line << endl;	// Output line
					line = "";	// Clear line
				}
				else if (readBuffer[0] == '~')
				{
					break;
				}
				else
				{
					line += readBuffer[0];	// Else add symbol to line
				}
			}
		}

		cout << "child stop" << endl;
		return 0;
	}
}