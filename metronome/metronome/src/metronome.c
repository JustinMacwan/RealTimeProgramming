#include "metronome.h"

metronome_t metronome_obj;
name_attach_t *attach;
int server_coid;
char data[512];

int main(int argc, char *argv[]) {

	dispatch_t* dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t conn_funcs;
	ioattr_t ioattr[NumDevices];
	pthread_attr_t attr;
	dispatch_context_t *ctp;
	int id;

	if (argc != 4) {
		fprintf(stderr, "ERROR: You must provide all required arguments.\n");
		show_usage();
		exit(EXIT_FAILURE);
	}

	metronome_obj.bpm = atoi(argv[1]);
	metronome_obj.tst = atoi(argv[2]);
	metronome_obj.tsb = atoi(argv[3]);

	iofunc_funcs_t metocb_funcs = { _IOFUNC_NFUNCS, metocb_calloc, metocb_free };

	iofunc_mount_t metocb_mount = { 0, 0, 0, 0, &metocb_funcs };

	if ((dpp = dispatch_create()) == NULL) {
		fprintf(stderr, "%s:  Unable to allocate dispatch context.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &conn_funcs, _RESMGR_IO_NFUNCS, &io_funcs);
	conn_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	for (int i = 0; i < NumDevices; i++) {
		iofunc_attr_init(&ioattr[i].attr, S_IFCHR | 0666, NULL, NULL);
		ioattr[i].device = i;
		ioattr[i].attr.mount = &metocb_mount;
		if ((id = resmgr_attach(dpp, NULL, devnames[i], _FTYPE_ANY, 0, &conn_funcs, &io_funcs, &ioattr[i])) == -1) {
			fprintf(stderr, "%s:  Unable to attach name.\n", argv[0]);
			return (EXIT_FAILURE);
		}
	}
	ctp = dispatch_context_alloc(dpp);

	pthread_attr_init(&attr);
	pthread_create(NULL, &attr, &metronome_thread, &metronome_obj);

	while (1) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}

	pthread_attr_destroy(&attr);
	name_detach(attach, 0);
	name_close(server_coid);

	return EXIT_SUCCESS;
}

void show_usage() {
	printf(
			"Command: ./metronome <beats/minute> <time-signature-top> <time-signature-bottom>\n");
}

int io_read(resmgr_context_t *ctp, io_read_t *msg, metocb_t *mocb) {

	int index;
	int nb;

	if (data == NULL)
		return 0;

	if (mocb->ocb.attr->device == HELP) {
		sprintf(data,
				"Metronome Resource Manager (Resmgr)\n\nUsage: metronome <bpm> <ts-top> <ts-bottom>\n\nAPI:\n pause[1-9]\t\t\t-pause the metronome for 1-9 seconds\n quit:\t\t\t\t- quit the metronome\n set <bpm> <ts-top> <ts-bottom>\t- set the metronome to <bpm> ts-top/ts-bottom\n start\t\t\t\t- start the metronome from stopped state\n stop\t\t\t\t- stop the metronome; use 'start' to resume\n");
	} else {

		index = table_lookup(&metronome_obj);

		sprintf(data, "[metronome: %d beats/min, time signature: %d/%d, sec-per-interval: %.2f, nanoSecs: %.0lf]\n",
				metronome_obj.bpm, t[index].tstop, t[index].tsbot, metronome_obj.timer.interval, metronome_obj.timer.nano_sec);
	}
	nb = strlen(data);

	//test to see if we have already sent the whole message.
	if (mocb->ocb.offset == nb)
		return 0;

	//We will return which ever is smaller the size of our data or the size of the buffer
	nb = min(nb, msg->i.nbytes);

	//Set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nb);

	//Copy data into reply buffer.
	SETIOV(ctp->iov, data, nb);

	//update offset into our data used to determine start position for next read.
	mocb->ocb.offset += nb;

	//If we are going to send any bytes update the access time for this resource.
	if (nb > 0)
		mocb->ocb.flags |= IOFUNC_ATTR_ATIME;

	return (_RESMGR_NPARTS(1));
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, metocb_t *mocb) {

	int nb = 0;

	if (mocb->ocb.attr->device == HELP) {
		printf("\nError: Cannot Write to device /dev/local/metronome-help\n");
		nb = msg->i.nbytes;
		_IO_SET_WRITE_NBYTES(ctp, nb);
		return (_RESMGR_NPARTS(0));
	}

	if (msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg))) {

		char *buf;
		char * message;
		int small_integer = 0;
		buf = (char *) (msg + 1);

		if (strstr(buf, "pause") != NULL) {
			for (int i = 0; i < 2; i++) {
				message = strsep(&buf, " ");
			}
			small_integer = atoi(message);
			if (small_integer >= 1 && small_integer <= 9) {
				MsgSendPulse(server_coid, SchedGet(0, 0, NULL), PAUSE_PULSE, small_integer);
			} else {
				printf("Please enter a number between 1 and 9.\n");
			}

		} else if (strstr(buf, "quit") != NULL) {
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL), QUIT_PULSE, small_integer);
		} else if (strstr(buf, "start") != NULL) {
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL), START_PULSE, small_integer);
		} else if (strstr(buf, "stop") != NULL) {
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL), STOP_PULSE, small_integer);

		} else if (strstr(buf, "set") != NULL) {
			for (int i = 0; i < 4; i++) {
				message = strsep(&buf, " ");

				if (i == 1) {
					metronome_obj.bpm = atoi(message);
				} else if (i == 2) {
					metronome_obj.tst = atoi(message);
				} else if (i == 3) {
					metronome_obj.tsb = atoi(message);
				}
			}

			MsgSendPulse(server_coid, SchedGet(0, 0, NULL), SET_PULSE, small_integer);

		} else {
			printf("\nInvalid Command\n");
			strcpy(data, buf);
		}

		nb = msg->i.nbytes;
	}

	_IO_SET_WRITE_NBYTES(ctp, nb);

	if (msg->i.nbytes > 0)
		mocb->ocb.flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS(0));
}

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra) {

	if ((server_coid = name_open("metronome", 0)) == -1) {
		perror("ERROR - opening namespace failed \n ");
		return EXIT_FAILURE;
	}
	return (iofunc_open_default(ctp, msg, &handle->attr, extra));
}

void *metronome_thread() {
	struct sigevent event;
	struct itimerspec itime;
	timer_t timer_id;
	my_message_t msg;
	int rcvid;
	int index = 0;
	int status;
	char *tp;

	if ((attach = name_attach(NULL, "metronome", 0)) == NULL) {
		perror("Error - name_attach - ./metronome.c ");
		exit(EXIT_FAILURE);
	}

	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, attach->chid, _NTO_SIDE_CHANNEL, 0);
	event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
	event.sigev_code = MET_PULSE;

	timer_create(CLOCK_REALTIME, &event, &timer_id);

	index = table_lookup(&metronome_obj);

	if (index == -1) {
		perror("\nStopped\nPlease set a valid measure.");
		stop_timer(&itime, timer_id);
		status = STOPPED;
	} else {
		set_timer(&metronome_obj);
		start_timer(&itime, timer_id, &metronome_obj);
	}

	tp = t[index].pattern;

	for (;;) {
		if ((rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL)) == -1) {
			perror("Error receiving message.");
			exit(EXIT_FAILURE);
		}

		if (rcvid == 0) {
			switch (msg.pulse.code) {
			case MET_PULSE:
				if (*tp == '|') {
					printf("%.2s", tp);
					tp += 2;
				} else if (*tp == '\0') {
					printf("\n");
					tp = t[index].pattern;
				} else {
					printf("%c", *tp++);
				}
				break;
			case START_PULSE:
				if (status == STOPPED) {
					start_timer(&itime, timer_id, &metronome_obj);
					status = STARTED;
				}
				break;
			case STOP_PULSE:
				if (status == STARTED || status == PAUSED) {
					stop_timer(&itime, timer_id);
					status = STOPPED;
				}
				break;
			case PAUSE_PULSE:
				if (status == STARTED) {
					itime.it_value.tv_sec = msg.pulse.value.sival_int;
					timer_settime(timer_id, 0, &itime, NULL);
				}
				break;
			case SET_PULSE:
				index = table_lookup(&metronome_obj);
				tp = t[index].pattern;
				set_timer(&metronome_obj);
				start_timer(&itime, timer_id, &metronome_obj);
				status = STARTED;
				printf("\n");
				break;
			case QUIT_PULSE:
				timer_delete(timer_id);
				name_detach(attach, 0);
				name_close(server_coid);
				exit(EXIT_SUCCESS);
			}
		}
		fflush(stdout);
	}
	return NULL;
}

void set_timer(metronome_t *metronome_obj) {
	metronome_obj->timer.length = (double) 60 / metronome_obj->bpm;
	metronome_obj->timer.measure = metronome_obj->timer.length * 2;
	metronome_obj->timer.interval = metronome_obj->timer.measure / metronome_obj->tsb;
	metronome_obj->timer.nano_sec = (metronome_obj->timer.interval - (int) metronome_obj->timer.interval) * 1e+9;
}

int table_lookup(metronome_t *metronome_obj) {
	int i = 0;
	for (i = 0; i < 8; i++) {
		if (t[i].tstop == metronome_obj->tst && t[i].tsbot == metronome_obj->tsb) {
			return i;
		}
	}
	return -1;
}

void stop_timer(struct itimerspec *itime, timer_t timer_id) {
	itime->it_value.tv_sec = 0;
	timer_settime(timer_id, 0, itime, NULL);
}

void start_timer(struct itimerspec *itime, timer_t timer_id,
		metronome_t *metronome_obj) {
	itime->it_value.tv_sec = 1;
	itime->it_value.tv_nsec = 0;
	itime->it_interval.tv_sec = metronome_obj->timer.interval;
	itime->it_interval.tv_nsec = metronome_obj->timer.nano_sec;
	timer_settime(timer_id, 0, itime, NULL);
}

metocb_t *metocb_calloc(resmgr_context_t *ctp, ioattr_t *ioattr) {
	metocb_t *metocb;
	metocb = calloc(1, sizeof(metocb_t));
	metocb->ocb.offset = 0;
	return metocb;
}

void metocb_free(metocb_t *metocb) {
	free(metocb);
}
