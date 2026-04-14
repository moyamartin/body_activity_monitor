#include "API_cmdparser.h"
#include "API_uart.h"

#define LINE_FEED 		'\n'
#define CARRIAGE_RETURN '\r'
#define EOL				'\0'

static cmd_fsm_state_t cmdParserFSM;
static uint8_t rx_buffer[MAX_RX_SIZE];
static uint16_t idx;
static cmd_status_t cmdParserStatus;
static bool_t cmdParserInitialzed = false;


static print_help(const char *)
{
	uartSendString("Usage:");
}

static cmd_t cmds[] = {
		{.cmd = "HELP", .func = print_help }
};

static bool_t cmdParserReset() {
	idx = 0;
	memset(rx_buffer, 0, sizeof(rx_buffer)/sizeof(rx_buffer)[0]);
	cmdParserFSM = CMD_IDLE;
}

static bool_t cmdProcessLine(const char* cmd) {
	size_t i;
	for(i = 0; i < sizeof(cmds) / sizeof(cmd[0]); i++) {
		if(strcmp(cmd, cmds[i].cmd) == 0) {
			cmd.func(args);
			return true;
		}
	}
	/// not found
	return false;
}

void cmdParserInit(void)
{
	cmdParserFSM = CMD_IDLE;
	cmdParserStatus = CMD_OK;
	uartInit();
	idx = 0;
	cmdParserInitialized = true;
}

void cmdPoll(void)
{
	uint8_t c;
	if(uartReceiveStringSize(&c, 1)) {
		switch(cmdParserFSM){
		case CMD_IDLE:
			if(c != EOL && c != CARRIAGE_RETURN && c!= LINE_FEED) {
				cmdParserFSM = CMD_RECEIVING;
				rx_buffer[idx++] = c;
			}
			break;
		}
		case CMD_RECEIVING:
			if(idx == MAX_RX_SIZE && (c != CARRIAGE_RETURN || c != LINE_FEED)) {
				cmdParserFSM = CMD_ERROR;
				cmdParserStatus = CMD_ERR_OVERFLOW;
			}
			if(c == EOL) {
				cmdParserFSM = CMD_ERROR;
				cmdParserStatus = CMD_ERR_TERMINATED;
			}
			rx_buffer[idx++] = c;
			if(c == CARRIAGE_RETURN || c == LINE_FEED) {
				cmdParserFSM = CMD_PROCESS;
			}
		case CMD_PROCESS:
			// process stuff
			if(rx_buffer[0] == '#' || strncmp(rx_buffer, "//", 2) == 0) {
				cmdParserStatus = CMD_ERR_IGNORED;
				cmdParserFSM = CMD_ERROR;
				idx = 0;
			}
			break;
		case CMD_EXEC:
			break;
		case CMD_ERROR:
			// print error msg
			switch(cmdParserStatus){
			case CMD_ERR_OVERFLOW:
				uartSendString("ERROR: line too long\r\n");
				break;
			case CMD_ERR_IGNORED:
				uartSendString("ERROR: Comments (beginning with # or //) are ignored");
				break;
			case CMD_ERR_SYNTAX:
				uartSendString("ERROR: Wrong syntax\r\n");
				break;
			case
			}
			uartSendString()


	}
}
