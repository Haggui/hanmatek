/*
 * icmpCount_test.c
 *
 *  Created on: 19 Aug. 2020
 *      Author: haggui
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  /* Many POSIX functions (but not all, by a large margin) */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <glib.h>
#include <readline/readline.h>
#include <readline/history.h>

#define ICMP_IOC_MAGIC 'k' /* "magic number" 8 bits à choisir */
#define ICMP_START_COUNT _IOW(ICMP_IOC_MAGIC, 128, int)
#define ICMP_STOP_COUNT  _IOW(ICMP_IOC_MAGIC, 129, int)
#define ICMP_RESET_COUNT _IOW(ICMP_IOC_MAGIC, 130, int)


static void cmd_help(int argcp, char **argvp);
static void cmd_exit(int argcp, char **argvp);
static void powerSwitch(int argcp, char **argvp);
static void protectVoltage(int argcp, char **argvp);
static void setVoltage(int argcp, char **argvp);

static struct {
	const char *cmd;
	void (*func)(int argcp, char **argvp);
	const char *params;
	const char *desc;
} commands[] = {
	{ "help",		cmd_help,	"",
		"Show this help"},
	{ "Exit",		cmd_exit,	"",
		"Exit interactive mode" },
    { "Power-Switch",	powerSwitch,	"<0/1>",
			"Power On/Off" },
    { "Set-Voltage",	setVoltage,	"<voltage>",
			"set Voltage to a defined value" },
    { "Protect-Voltage",	protectVoltage,	"<voltage>",
			"Protect Voltage from being changed" },
    { NULL, NULL, NULL}
};

static int serial_terminal;
static GMainLoop *event_loop;
static GString *prompt;

static void cmd_help(int argcp, char **argvp)
{
	int i;

	for (i = 0; commands[i].cmd; i++)
		printf("%-20s %-11s %s\n", commands[i].cmd,
				commands[i].params, commands[i].desc);
}

static void cmd_exit(int argcp, char **argvp)
{
	rl_callback_handler_remove();
	g_main_loop_quit(event_loop);
}

static void powerSwitch(int argcp, char **argvp)
{
	char write_buffer[32];   /* Buffer to store the data received */
	int value;
	if(argcp <2)
	{
		perror("powerSwitch <val>\n");
		return;
	}
	value = atoi(argvp[1]);
	write_buffer[0] = 0x01;
	write_buffer[1] = value ? 1 : 0;
	read(serial_terminal, write_buffer,2); /* Read the data */ 
}

static void protectVoltage(int argcp, char **argvp)
{
	char write_buffer[32];   /* Buffer to store the data received */
	int value;
	if(argcp <2)
	{
		perror("powerSwitch <val>\n");
		return;
	}
	write_buffer[0] = 0x20;
	value = round(atof(argvp[1])*100);
	memcpy(&write_buffer[1], &value, sizeof(int));
	read(serial_terminal, write_buffer, sizeof(int) +1); /* Read the data */ 
}


static void setVoltage(int argcp, char **argvp)
{
	char write_buffer[32];   /* Buffer to store the data received */
	int value;
	if(argcp <2)
	{
		perror("powerSwitch <val>\n");
		return;
	}
	write_buffer[0] = 0x30;
	value = round(atof(argvp[1])*100);
	memcpy(&write_buffer[1], &value, sizeof(int));
	read(serial_terminal, write_buffer, sizeof(int) +1); /* Read the data */ 
}


static char *completion_generator(const char *text, int state)
{
	static int index = 0, len = 0;
	const char *cmd = NULL;

	if (state == 0) {
		index = 0;
		len = strlen(text);
	}

	while ((cmd = commands[index].cmd) != NULL) {
		index++;
		if (strncmp(cmd, text, len) == 0)
			return strdup(cmd);
	}

	return NULL;
}

static char **commands_completion(const char *text, int start, int end)
{
	if (start == 0)
		return rl_completion_matches(text, &completion_generator);
	else
		return NULL;
}

static char *get_prompt(void)
{
	g_string_append(prompt, "> ");
	return prompt->str;
}

static gboolean prompt_read(GIOChannel *chan, GIOCondition cond,
							gpointer user_data)
{
	if (cond & (G_IO_HUP | G_IO_ERR | G_IO_NVAL)) {
		g_io_channel_unref(chan);
		return FALSE;
	}

	rl_callback_read_char();

	return TRUE;
}

static void parse_line(char *line_read)
{
	gchar **argvp;
	int argcp;
	int i;

	if (line_read == NULL) {
		printf("\n");
		cmd_exit(0, NULL);
		return;
	}

	line_read = g_strstrip(line_read);

	if (*line_read == '\0')
		return;

	add_history(line_read);

	g_shell_parse_argv(line_read, &argcp, &argvp, NULL);

	for (i = 0; commands[i].cmd; i++)
		if (strcasecmp(commands[i].cmd, argvp[0]) == 0)
			break;

	if (commands[i].cmd)
		commands[i].func(argcp, argvp);
	else
		printf("%s: command not found\n", argvp[0]);

	g_strfreev(argvp);
}

static int open_terminal(void)
{
	serial_terminal = open("/dev/ttyUSB0",O_RDWR | O_NOCTTY);	/* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
			   					/* O_RDWR   - Read/Write access to serial port       */
								/* O_NOCTTY - No terminal will control the process   */
								/* Open in blocking mode,read will wait*/
	if(serial_terminal == -1){				/* Error Checking */
        fprintf(stderr, "\n  Error! in Opening ttyUSB0 ");
		return -1;
	}
	/*---------- Setting the Attributes of the serial port using termios structure --------- */
		
	struct termios SerialPortSettings;	/* Create the structure                          */

	tcgetattr(serial_terminal, &SerialPortSettings);	/* Get the current attributes of the Serial port */

	/* Setting the Baud rate */
	cfsetispeed(&SerialPortSettings,B9600); /* Set Read  Speed as 9600                       */
	cfsetospeed(&SerialPortSettings,B9600); /* Set Write Speed as 9600                       */

	/* 8N1 Mode */
	SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
	SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
	SerialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
	SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */
		
	SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
	SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */ 
		
		
	SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
	SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */

	SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/
		
	/* Setting Time outs */
	SerialPortSettings.c_cc[VMIN] = 10; /* Read at least 10 characters */
	SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */


	if((tcsetattr(serial_terminal,TCSANOW,&SerialPortSettings)) != 0){ /* Set the attributes to the termios structure*/
		printf("\n  ERROR ! in Setting attributes");
		return -1;
	}			
	/*------------------------------- Read data from serial port -----------------------------*/

	tcflush(serial_terminal, TCIFLUSH);   /* Discards old data in the rx buffer            */
	return 0;
}

int main(int argc, char* argv[])
{
	GIOChannel *pchan;
	gint events;
	if(open_terminal() != 0){
		return -1;
	}
	prompt = g_string_new(NULL);
	event_loop = g_main_loop_new(NULL, false);

	pchan = g_io_channel_unix_new(fileno(stdin));
	g_io_channel_set_close_on_unref(pchan, TRUE);
	events = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
	g_io_add_watch(pchan, events, prompt_read, NULL);

	rl_attempted_completion_function = commands_completion;
	rl_callback_handler_install(get_prompt(), parse_line);
	g_main_loop_run(event_loop);

	rl_callback_handler_remove();
	g_io_channel_unref(pchan);
	g_main_loop_unref(event_loop);
	g_string_free(prompt, TRUE);
	close(serial_terminal); /* Close the serial port */
	return 0;
}


