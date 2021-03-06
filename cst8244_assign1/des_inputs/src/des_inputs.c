#include <errno.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <errno.h>
#include <sys/dispatch.h>
#include "../../des_controller/src/des.h"

void show_usage(void);

int main(int argc, char* argv[]) {

	int coid;
	person_t person;
	controller_response_t controller_response;

	/* Connect to controller */
	if((coid = name_open("des_controller", 0)) == -1){
		fprintf (stderr, "ERROR: Could not connect to the controller\n");
		exit(EXIT_FAILURE);
	}

	while(1) {
		char  input [5];
		printf("\n Enter the event type (ls = left scan, rs = right scan, ws = weight scale, lo = left open, \n"
				"ro = right open, lc = left closed, rc = right closed, gru = guard right unlock, grl = guard right lock, \n"
				"gll = guard left lock, glu = guard left unlock, exit = exit programs) \n");
		scanf(" %s",input);

		if(strncmp(input,inMessage[IN_LEFT_SCAN],strlen(inMessage[IN_LEFT_SCAN])) == 0){
			person.state = ST_LEFT_SCAN; /* SET LEFT SCAN STATE */
			printf("Enter ID:\n"); /* enter ID */
			scanf("%d", &person.id);
		}else if(strncmp(input,inMessage[IN_RIGHT_SCAN],strlen(inMessage[IN_RIGHT_SCAN])) == 0){
			person.state = ST_RIGHT_SCAN;/* SET RIGHT SCAN STATE */
			printf("Enter ID:\n"); /* enter ID */
			scanf("%d", &person.id);
		}else if(strncmp(input,inMessage[IN_WEIGHT_SCALE],strlen(inMessage[IN_WEIGHT_SCALE])) == 0){
			person.state = ST_WEIGHT_SCALE; /* SET WEIGHT SCALE STATE */
			printf("Please Enter your Weight\n");
			scanf("%d",&person.weight);
		}else if(strncmp(input,inMessage[IN_LEFT_OPEN],strlen(inMessage[IN_LEFT_OPEN])) == 0){
			person.state = ST_LEFT_OPEN; /* SET LEFT OPEN STATE */
		}
		else if(strncmp(input,inMessage[IN_RIGHT_OPEN],strlen(inMessage[IN_RIGHT_OPEN])) == 0){
			person.state = ST_RIGHT_OPEN; /* SET RIGHT OPEN STATE */
		}
		else if(strncmp(input,inMessage[IN_LEFT_CLOSED],strlen(inMessage[IN_LEFT_CLOSED])) == 0){
			person.state = ST_LEFT_CLOSED; /* SET LEFT CLOSE STATE */
		}
		else if(strncmp(input,inMessage[IN_RIGHT_CLOSED],strlen(inMessage[IN_RIGHT_CLOSED])) == 0){
			person.state = ST_RIGHT_CLOSED; /* SET RIGHT CLOSE STATE */
		}
		else if(strncmp(input,inMessage[IN_GUARD_RIGHT_LOCK],strlen(inMessage[IN_GUARD_RIGHT_LOCK])) == 0){
			person.state = ST_GUARD_RIGHT_LOCK; /* SET GUARD RIGHT LOCK */
		}
		else if(strncmp(input,inMessage[IN_GUARD_RIGHT_UNLOCK],strlen(inMessage[IN_GUARD_RIGHT_UNLOCK])) == 0){
			person.state = ST_GUARD_RIGHT_UNLOCK; /* SET GUARD RIGHT UNLOCK */
		}
		else if(strncmp(input,inMessage[IN_GUARD_LEFT_LOCK],strlen(inMessage[IN_GUARD_LEFT_LOCK])) == 0){
			person.state = ST_GUARD_LEFT_LOCK; /* SET GUARD LEFT LOCK */
		}
		else if(strncmp(input,inMessage[IN_GUARD_LEFT_UNLOCK],strlen(inMessage[IN_GUARD_LEFT_UNLOCK])) == 0){
			person.state = ST_GUARD_LEFT_UNLOCK; /* SET GUARD LEFT UNLOCK */
		}
		else if(strncmp(input,inMessage[IN_EXIT],strlen(inMessage[IN_EXIT])) == 0){
			person.state = ST_EXIT; /* SET EXIT STATE */
		}

		if(MsgSend(coid,&person,sizeof(person),&controller_response,sizeof(controller_response)) == -1){
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}

		if(sizeof(controller_response) == 0){
			fprintf(stderr, "ERROR: Got a zero-length response from the server.\n");
			exit(EXIT_FAILURE);
		}

		if (controller_response.statusCode != EOK) {
			printf("ERROR: Message from server: %s", controller_response.errorMsg);
		}
	}
	sleep(5);
	/* Disconnect from server */
	name_close(coid);
	return EXIT_SUCCESS;
}

void show_usage(void) {
	printf("   USAGE:\n");
	printf("./des_inputs <controller-pid>\n");
}
