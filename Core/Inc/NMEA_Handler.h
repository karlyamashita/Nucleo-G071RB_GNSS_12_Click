/*
 * NMEA_Handler.h
 *
 *  Created on: May 24, 2024
 *      Author: karl.yamashita
 */

#ifndef INC_NMEA_HANDLER_H_
#define INC_NMEA_HANDLER_H_


typedef struct
{
	char nmea[6]; // 5 char + null
	char time[10];
	char status[2];
	char lat[12];
	char ns[2];
	char lon[12];
	char ew[2];
	char spd[8];
	char cog[6];
	char date[7];
	char mv[8];
	char mvew[2];
	char posMode[8];
	char navStatus[2];
}GNRMC_t;

int NMEA_GNRMC_Data(char *msg);
int NMEA_GNRMC_Command(char *msg, char *retStr);

bool NMEA_CalculateChecksum(char *msg);

char *strtok_fr (char *s, char delim, char **save_ptr);
char *strtok_f (char *s, char delim);

#endif /* INC_NMEA_HANDLER_H_ */
