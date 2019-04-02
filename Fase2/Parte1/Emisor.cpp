#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace std;

int main (int argc, char **argv){
	printf("--beginning of program\n");

	int counter = 0;
	pid_t pid = fork();

	if (pid == 0){
		for(int i = 0; i < 5; ++i){
			printf("Child process: counter = %d\n", ++counter);
		}
	}else if (pid > 0) {
		for (int i = 0; i < 5; ++i){
			printf("parent process: counter = %d\n", ++counter);
		}
	}else {
		// fork failed
		printf("fork() failed!\n");
		return 1;
	}

	printf("--- end of program --\n");
	return 0;
}