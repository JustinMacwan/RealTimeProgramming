#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

volatile sig_atomic_t usr1Happened = 0;
void sig_handler();

int main(void) {

	pid_t pid;
	int numchildren;
	int rt;
	int proc = 0;

	struct sigaction sa;
	sa.sa_handler = sig_handler;
	sa.sa_flags = 0; // or SA_RESTART
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("Enter the number of children:");
	scanf("%d", &numchildren);

	printf("PID = %d : Parent Running...\n", getpid());

	while (proc < numchildren) {
		proc++;

		switch (pid = fork()) {

		case -1:
			perror("fork");
			exit(1);

		case 0:
			printf("PID = %d : Child running...\n", getpid());
			while (!usr1Happened) {
			}
			printf("PID = %d : Child Received USR1.\n", getpid());

			printf("PID = %d : Child exiting.\n", getpid());

			exit(EXIT_SUCCESS);
		}
	}
	while (numchildren > 0) {
		rt = wait(NULL);
		if (rt > 0)
			numchildren--;
	}

	printf("PID = %d : Children finished, parent exiting.\n", getpid());
	return EXIT_SUCCESS;
}

void sig_handler() {
	usr1Happened++;
}
