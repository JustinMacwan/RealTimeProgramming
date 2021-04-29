/*
 * des.h
 *
 *  Created on: Mar 13, 2021
 *      Author: Justin Macwan
 */

#ifndef DES_H_
#define DES_H_

#define LEFT 0
#define RIGHT 1
#define DEFAULT 2

struct person { /* SENDING person structure to server to process */
	int id;
	int weight;
	int state;
} typedef person_t;

struct controller_response { /* response structure from server, person included in response */
	person_t person;
	int statusCode;
	char errorMsg[128];
} typedef controller_response_t;

typedef void *(*StateF)();

#define NUM_STATES 15
typedef enum  /* all states possible in progra, */
{
	ST_START = 0, 				/* 0: Start state                */
	ST_READY = 1, 				/* 1: Ready state            */
	ST_LEFT_SCAN = 2, 			/* 2: Left_Scan state            */
	ST_RIGHT_SCAN = 3, 			/* 3: Right_Scan state           */
	ST_WEIGHT_SCALE = 4,		/* 4: Weighted state              */
	ST_LEFT_OPEN = 5, 			/* 5: Left_Open state            */
	ST_RIGHT_OPEN = 6, 			/* 6: Right_Open state           */
	ST_LEFT_CLOSED = 7,  		/* 7: Left_Close state           */
	ST_RIGHT_CLOSED = 8,		/* 8: Right_Close state          */
	ST_GUARD_RIGHT_LOCK = 9,	/* 9: Guard_Right_Lock state     */
	ST_GUARD_RIGHT_UNLOCK = 10,	/* 10: Guard_Right_Unlock state  */
	ST_GUARD_LEFT_LOCK = 11,	/* 11: Guard_Left_Lock state     */
	ST_GUARD_LEFT_UNLOCK = 12,	/* 12: Guard_Left_Unlock state   */
	ST_EXIT = 13,				/* 13: Exit State				 */
	ST_STOP = 14				/* 14: Stop State				 */
} State;

#define NUM_INPUTS 12
typedef enum
{
	IN_LEFT_SCAN = 0,			/* LEFT SCAN 			*/
	IN_RIGHT_SCAN = 1,			/* RIGHT SCAN 			*/
	IN_WEIGHT_SCALE = 2,		/* WEIGHT SCALE 		*/
	IN_LEFT_OPEN = 3,			/* LEFT OPEN 			*/
	IN_RIGHT_OPEN = 4,			/* RIGHT OPEN 			*/
	IN_LEFT_CLOSED = 5,			/* LEFT CLOSE 			*/
	IN_RIGHT_CLOSED = 6,		/* RIGHT CLOSE 			*/
	IN_GUARD_RIGHT_LOCK = 7,	/* GUARD RIGHT LOCKED 	*/
	IN_GUARD_RIGHT_UNLOCK = 8,	/* GUARD RIGHT UNLOCKED */
	IN_GUARD_LEFT_LOCK = 9,		/* GUARD LEFT LOCKED 	*/
	IN_GUARD_LEFT_UNLOCK = 10,	/* GUARD LEFT UNLOCKED 	*/
	IN_EXIT = 11				/* EXIT 				*/
} Input;

const char *inMessage[NUM_INPUTS] = {
		"ls",  /* IN_LS    */
		"rs",  /* IN_RS    */
		"ws",  /* IN_WS    */
		"lo",  /* IN_LO    */
		"ro",  /* IN_RO    */
		"lc",  /* IN_LC    */
		"rc",  /* IN_RC    */
		"grl", /* IN_GRL   */
		"gru", /* IN_GRU   */
		"gll", /* IN_GLL   */
		"glu", /* IN_GLU   */
		"exit" /* IN_EXIT  */
};

#define NUM_OUTPUTS 14
typedef enum
{
	OUT_START = 0,					/* START                */
	OUT_READY = 1,					/* READY                */
	OUT_SCAN = 2,					/* LEFT-RIGHT SCAN 		*/
	OUT_WEIGHT_SCALE = 3,			/* WEIGHT SCALE 		*/
	OUT_LEFT_OPEN = 4,				/* LEFT OPEN 			*/
	OUT_RIGHT_OPEN = 5,				/* RIGHT OPEN 			*/
	OUT_LEFT_CLOSED = 6,			/* LEFT CLOSED 			*/
	OUT_RIGHT_CLOSED = 7,			/* RIGHT CLOSED 		*/
	OUT_GUARD_RIGHT_LOCK = 8,		/* GUARD RIGHT LOCKED 	*/
	OUT_GUARD_RIGHT_UNLOCK = 9,	 	/* GUARD RIGHT UNLOCK 	*/
	OUT_GUARD_LEFT_LOCK = 10,		/* GUARD LEFT LOCKED 	*/
	OUT_GUARD_LEFT_UNLOCK = 11,		/* GUARD LEFT UNLOCK 	*/
	OUT_EXIT = 12,					/* EXIT 				*/
	OUT_STOP = 13					/* STOP 				*/
} Output;

const char *outMessage[NUM_OUTPUTS] = {
		"Controller PID: ",
		"Waiting for Person... ",
		"Person scanned ID. ID =",
		"Person weighed. Weight =",
		"Left door open.",
		"Right door open.",
		"Left door closed. ",
		"Right door closed. ",
		"Right door locked by Guard. ",
		"Right door unlocked by Guard. ",
		"Left door locked by Guard. ",
		"Left door unlocked by Guard. ",
		"Exit display."
		"Exiting Door Entry System..."
};

#endif /* DES_H_ */
