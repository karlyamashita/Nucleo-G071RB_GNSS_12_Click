/*
 * CommandList_Handler.c
 *
 *  Created on: May 1, 2025
 *      Author: karl.yamashita
 */

#include "main.h"


const Command_t help =
{
	.command = "help",
	.args = "help\r\n"
			"\thelp <single character color>",
	.notes = "r=red, g= green, b=blue, y=yellow, m=magenta, c=cyan, w=white ",
	.func = Command_List_Print
};

const Command_t gnrmc =
{
	.command = "gnrmc",
	.args = "gnrmc ?\r\n"
			"\tgnrmc status\r\n"
			"\tgnrmc time\r\n"
			"\tgnrmc date\r\n"
			"\tgnrmc lat\r\n"
			"\tgnrmc lon\r\n"
			"\tgnrmc spd",
	.notes = NULL,
	.func = NMEA_GNRMC_Command
};

const Command_t pass =
{
	.command = "pass",
	.args = "pass 0\r\n"
			"\tpass 1",
	.notes = NULL,
	.func = Passthrough
};

const Command_t version =
{
	.command = "version",
	.args = NULL,
	.notes = NULL,
	.func = Version
};





/*
 * Description: User shall add each instances of commands to array.
 */
const Command_t Command_List[] =
{
	help,
	gnrmc,
	pass,
	version
};

// need to place here for CommandList_Drv.c while extern placed in header file.
int CommandList_array_size = sizeof(Command_List);


