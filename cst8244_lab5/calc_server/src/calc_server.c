#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>

#include "calc_message.h"

int main(void) {
	client_send_t msg_send;
	server_response_t msg_receive;
	int chid, rcvid;

	chid = ChannelCreate(0);
	if (chid == -1) {
		fprintf(stderr, "Could not create channel\n");
		exit(EXIT_FAILURE);
	}

	printf("CalcServer PID: %d\n", getpid());
	while(1){
		rcvid = MsgReceive(chid, (void*) &msg_send, sizeof(msg_send), NULL);
		msg_receive.statusCode = SRVR_OK;

		switch (msg_send.operator) {
		case '+':
			if (((msg_send.right_hand > 0)
					&& (msg_send.left_hand > INT_MAX - msg_send.right_hand))
					|| ((msg_send.right_hand < 0)
							&& (msg_send.left_hand < INT_MIN + msg_send.right_hand))) {
				msg_receive.statusCode = SRVR_OVERFLOW;
				sprintf(msg_receive.errorMsg, "OVERFLOW: %d + %d", msg_send.left_hand,
						msg_send.right_hand);
				break;
			}
			msg_receive.answer = msg_send.left_hand + msg_send.right_hand;
			break;
		case '-':
			if (((msg_send.right_hand > 0)
					&& (msg_send.left_hand < INT_MIN + msg_send.right_hand))
					|| ((msg_send.right_hand < 0)
							&& (msg_send.left_hand > INT_MAX - msg_send.right_hand))) {
				msg_receive.statusCode = SRVR_OVERFLOW;
				sprintf(msg_receive.errorMsg, "OVERFLOW: %d - %d", msg_send.left_hand,
						msg_send.right_hand);
				break;
			}
			msg_receive.answer = msg_send.left_hand - msg_send.right_hand;
			break;
		case '/':
			if (msg_send.right_hand == 0) {
				msg_receive.statusCode = SRVR_UNDEFINED;
				sprintf(msg_receive.errorMsg, "UNDEFINED: %d / %d\n", msg_send.left_hand,
						msg_send.right_hand);
				break;
			}
			msg_receive.answer = (double) msg_send.left_hand / (double) msg_send.right_hand;
			break;
		case 'x':
			if ((msg_send.right_hand != 0
					&& (msg_send.left_hand > INT_MAX / msg_send.right_hand))
					|| (msg_send.right_hand != 0
							&& (msg_send.left_hand < INT_MIN / msg_send.right_hand))) {
				msg_receive.statusCode = SRVR_OVERFLOW;
				sprintf(msg_receive.errorMsg, "OVERFLOW: %d x %d", msg_send.left_hand,
						msg_send.right_hand);
				break;
			}
			msg_receive.answer = msg_send.left_hand * msg_send.right_hand;
			break;
		default:
			msg_receive.statusCode = SRVR_INVALID_OPERATOR;
			sprintf(msg_receive.errorMsg, "INVALID OPERATOR: %c\n", msg_send.operator);
		}

		MsgReply(rcvid, EOK, &msg_receive, sizeof(msg_receive));
	}

	ChannelDestroy(chid);
	return EXIT_SUCCESS;
}
