#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>

#include "../../calc_server/src/calc_message.h"

void usage();

int main(int argc, char* argv[]) {

	pid_t server_pid;
	client_send_t msg_send;
	server_response_t msg_receive;
	int conn;

	if (argc != 5) {
		usage();
		exit(EXIT_FAILURE);
	}

	server_pid = atoi(argv[1]);
	msg_send.left_hand = atoi(argv[2]);
	msg_send.operator = argv[3][0];
	msg_send.right_hand = atoi(argv[4]);

	conn = ConnectAttach(ND_LOCAL_NODE, server_pid, 1, _NTO_SIDE_CHANNEL, 0);
	if (conn == -1) {
		fprintf(stderr, "Couldn't connect to processID %d\n", server_pid);
		exit(EXIT_FAILURE);
	}

	if (MsgSend(conn, &msg_send, sizeof(msg_send), &msg_receive, sizeof(msg_receive)) == -1) {
		fprintf(stderr, "Msg had an error\n");
		exit(EXIT_FAILURE);
	}

	if (msg_receive.statusCode == SRVR_OK) {
		printf("The server has calculated the result of %d %c %d as %.2f\n",
				msg_send.left_hand, msg_send.operator, msg_send.right_hand, msg_receive.answer);
	} else {
		printf("Error message from server: %s\n", msg_receive.errorMsg);
	}

	ConnectDetach(conn);
	return EXIT_SUCCESS;
}

void usage() {
	printf(
			"Usage: ./calc_client <server-pid> <left-hand> <operator> <right-hand>\n");
}
