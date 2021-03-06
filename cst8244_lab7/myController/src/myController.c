#include <stdio.h>
#include <stdlib.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/resmgr.h>

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL

typedef union {
	struct _pulse   pulse;
	char msg[255];
} my_message_t;

int main(void) {
	name_attach_t *attach;
	FILE *file;
	char status[255];
	char value[255];
	int rcvid;
	my_message_t msg;

	if ((attach = name_attach(NULL, "mydevice", 0)) == NULL){
		perror("Could not connect to mydevice");
		return EXIT_FAILURE;
	}

	if ((file = fopen("/dev/local/mydevice", "r+")) == NULL){
		printf("Error opening file\n");
		exit (EXIT_FAILURE);
	}
	fscanf(file, "%s%s", status, value);

	if (strcmp(status, "status") == 0){
		printf("%s", value);
	}
	if (strcmp(value, "closed") == 0){
		name_detach(attach, 0);
		fclose(file);
		return EXIT_SUCCESS;
	}
	fclose(file);

	while(1) {
		if ((rcvid = MsgReceivePulse(attach->chid, &msg.msg, sizeof(msg.msg), NULL)) == -1){
			printf("Message Receive Pulse failure.\n");
			exit(EXIT_FAILURE);
		}

		if (rcvid == 0 && msg.pulse.code == MY_PULSE_CODE){
			printf("Small Integer: %d\n", msg.pulse.value.sival_int);

			if ((file = fopen("/dev/local/mydevice", "r+")) == NULL){
				printf("Error opening file\n");
				exit (EXIT_FAILURE);
			}

			fscanf(file, "%s%s", status, value);

			if (strcmp(status, "status") == 0){
				printf("Status %s\n", value);
			}
			if (strcmp(value, "closed") == 0){
				name_detach(attach, 0);
				fclose(file);
				return EXIT_SUCCESS;
			}
			fclose(file);
		} else {
			perror("Message Receive Pulse failure.\n");
			return EXIT_FAILURE;
		}
	}
	name_detach(attach,0);
	return EXIT_SUCCESS;
}

