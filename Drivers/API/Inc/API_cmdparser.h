#ifndef API_CMDPARSER_H_
#define API_CMDPARSER_H_

#define CMD_MAX_LINE 64 	// include EOL character ('\0')
#define CMD_MAX_TOKENS 3	// CMDS + two characters tops

/**
 * @brief representation of cmd parser status
 */
typedef enum {
	CMD_OK = 0,
	CMD_ERR_OVERFLOW,
	CMD_ERR_IGNORED,
	CMD_ERR_SYNTAX,
	CMD_ERR_UNKNOWN,
	CMD_ERR_EOL_RECEIVED,
	CMD_ERR_ARG,
} cmd_status_t;

/**
 * @brief representation of cmd parser FSM states
 */
typedef enum {
	CMD_IDLE = 0,
	CMD_RECEIVING,
	CMD_PROCESS,
	CMD_EXEC,
	CMD_ERROR,
} cmd_fsm_state_t;

/**
 * @brief representation of a cmd
 */
typedef struct {
	char cmd[];
	void (*func)(const char *arg);
} cmd_t;

/**
 * @brief initialize commander parser system
 */
void cmdParserInit(void);

/**
 * @brief updates parser's state machine.
 *
 * @note must be called periodically from main's superloop.
 * It process up to 16 bytes per invocation (non-blocking).
 */
void cmdPoll(void);

/**
 * @brief sends over UART the list of available cmds
 */
void cmdPrintHelp(void);

#endif /* API_CMDPARSER_H_ */
