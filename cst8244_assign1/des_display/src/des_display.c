#include <errno.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <errno.h>
#include <sys/dispatch.h>

#include "../../des_controller/src/des.h"

int main(void) {

	person_t person;
	controller_response_t controller_response;

	int rcvid;

	name_attach_t *attach;

	if ((attach = name_attach(NULL, "des_display", 0)) == NULL)
	{
		fprintf(stderr, "ERROR: Unable to create Channel.\n");
		exit(EXIT_FAILURE);
	}

	printf("Display PID = %d\n", getpid());

	while(person.state != ST_EXIT) {
		rcvid = MsgReceive(attach->chid, (void*)&person, sizeof(person), NULL);

		switch (person.state) {
		case ST_LEFT_SCAN:
		case ST_RIGHT_SCAN:
			printf("%s %d \n", outMessage[OUT_SCAN], person.id);
			break;
		case ST_WEIGHT_SCALE:
			printf("%s %d \n", outMessage[OUT_WEIGHT_SCALE], person.weight);
			break;
		case ST_READY:
			printf("%s \n", outMessage[OUT_READY]);
			break;
		case ST_LEFT_OPEN:
			printf("%s \n", outMessage[OUT_LEFT_OPEN]);
			break;
		case ST_RIGHT_OPEN:
			printf("%s \n", outMessage[OUT_RIGHT_OPEN]);
			break;
		case ST_LEFT_CLOSED:
			printf("%s \n", outMessage[OUT_LEFT_CLOSED]);
			break;
		case ST_RIGHT_CLOSED:
			printf("%s \n", outMessage[OUT_RIGHT_CLOSED]);
			break;
		case ST_GUARD_RIGHT_LOCK:
			printf("%s \n", outMessage[OUT_GUARD_RIGHT_LOCK]);
			break;
		case ST_GUARD_RIGHT_UNLOCK:
			printf("%s \n", outMessage[OUT_GUARD_RIGHT_UNLOCK]);
			break;
		case ST_GUARD_LEFT_LOCK:
			printf("%s \n", outMessage[OUT_GUARD_LEFT_LOCK]);
			break;
		case ST_GUARD_LEFT_UNLOCK:
			printf("%s \n", outMessage[OUT_GUARD_LEFT_UNLOCK]);
			break;
		case ST_EXIT:
			printf("%s \n", outMessage[OUT_EXIT]);
			printf("%s \n", outMessage[OUT_STOP]);
			break;
		default:
			printf("Invalid input\n");
			break;
		}

		MsgReply(rcvid, EOK, &controller_response, sizeof(controller_response));
	}
	sleep(5);

	if (controller_response.statusCode != EOK) {
		printf("ERROR: %s", controller_response.errorMsg);
	}

	name_detach(attach, 0);
	return EXIT_SUCCESS;
}
