#include <errno.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <errno.h>
#include <sys/dispatch.h>

#include "des.h"

void *ready_state();
void *left_scan();
void *right_scan();
void *weight_scale();
void *left_open();
void *right_open();
void *left_close();
void *right_close();
void *guard_right_lock();
void *guard_right_unlock();
void *guard_left_lock();
void *guard_left_unlock();
void *exit_state();

void show_usage(void);

controller_response_t controller_response;
person_t person;
int coid,rcvid;
StateF statefunc = ready_state;
int lr = DEFAULT;

int main(int argc, char* argv[]) {

	name_attach_t *attach;

	if((attach = name_attach(NULL, "des_controller", 0)) == NULL){
		fprintf(stderr, "ERROR: Unable to create Channel.\n");
		exit(EXIT_FAILURE);
	}

	if((coid = name_open("des_display", 0)) == -1){
		fprintf(stderr, "ERROR: Unable to connect to display.\n");
		exit(EXIT_FAILURE);
	}


	// print pid
	printf("Controller PID: %d \n",getpid());
	person.id = 0;
	person.weight = 0;
	person.state = ST_READY;
	while(1) {

		if((rcvid = MsgReceive(attach->chid,&person,sizeof(person),NULL)) == -1){ /* receive message from client */
			fprintf (stderr, "ERROR: Messsage not received.\n");; /* ON FAIL */
			exit(EXIT_FAILURE);
		}

		if (person.state == ST_EXIT) {
			exit_state();
		}

		statefunc = (StateF)(*statefunc)();

		controller_response.statusCode = EOK;
		MsgReply(rcvid, EOK, &controller_response, sizeof(controller_response));
		if (person.state == ST_EXIT) {
			break;
		}
	}
	name_close(coid);
	name_detach(attach, 0);
	return EXIT_SUCCESS;
}

void *ready_state() {

	if (person.state == ST_LEFT_SCAN) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		lr = LEFT;
		return left_scan;
	}

	if (person.state == ST_RIGHT_SCAN) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		lr = RIGHT;
		return right_scan;
	}
	return ready_state;
}

void *left_scan() {
	if (person.state == ST_GUARD_LEFT_UNLOCK) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return guard_left_unlock;
	}
	return left_scan;
}

void *right_scan() {
	if (person.state == ST_GUARD_RIGHT_UNLOCK) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return guard_right_unlock;
	}
	return right_scan;
}

void *weight_scale() {
	if (person.state == ST_LEFT_CLOSED) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return left_close;
	} else if (person.state == ST_RIGHT_CLOSED) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return right_close;
	}
	return weight_scale;
}
void *left_open() {
	if (person.state == ST_WEIGHT_SCALE) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return weight_scale;
	}
	else if (person.state == ST_LEFT_CLOSED) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return left_close;
	}
	return left_open;
}

void *right_open() {
	if (person.state == ST_WEIGHT_SCALE) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return weight_scale;
	}

	else if (person.state == ST_RIGHT_CLOSED) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return right_close;
	}
	return right_open;
}

void *left_close() {
	if (person.state == ST_GUARD_LEFT_LOCK) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return guard_left_lock;
	}
	return left_close;
}

void *right_close() {
	if (person.state == ST_GUARD_RIGHT_LOCK) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return guard_right_lock;
	}
	return right_close;
}

void *guard_right_lock() {
	if (lr == RIGHT && person.state == ST_GUARD_LEFT_UNLOCK) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		lr = DEFAULT;
		return guard_left_unlock;
	}
	if (lr == DEFAULT && person.state == ST_RIGHT_SCAN) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		lr = RIGHT;
		return right_scan;
	}
	if (lr == DEFAULT && person.state == ST_LEFT_SCAN) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		lr = LEFT;
		return left_scan;
	}
	return guard_right_lock;
}

void *guard_right_unlock() {
	if (person.state == ST_RIGHT_OPEN) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return right_open;
	}

	return guard_right_unlock;
}

void *guard_left_lock() {
	if (lr == LEFT && person.state == ST_GUARD_RIGHT_UNLOCK) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		lr = DEFAULT;
		return guard_right_unlock;
	}
	if (lr == DEFAULT && person.state == ST_RIGHT_SCAN) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		lr = RIGHT;
		return right_scan;
	}
	if (lr == DEFAULT && person.state == ST_LEFT_SCAN) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1){
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		lr = LEFT;
		return left_scan;
	}
	return guard_left_lock;
}

void *guard_left_unlock() {
	if (person.state == ST_LEFT_OPEN) {
		if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
			fprintf(stderr, "ERROR: Could not send message.\n");
			exit(EXIT_FAILURE);
		}
		return left_open;
	}

	return guard_left_unlock;
}

void *exit_state() {
	person.state = ST_EXIT;
	if (MsgSend(coid, &person, sizeof(person), &controller_response, sizeof(controller_response)) == -1) {
		fprintf(stderr, "ERROR: Could not send message.\n");
		exit(EXIT_FAILURE);
	}
	sleep(5);
	person.state = ST_STOP;
	return exit_state;
}

void show_usage(void) {
	printf("   USAGE:\n");
	printf("./des_controller <display-pid>\n");
}
