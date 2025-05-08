/*
 * NMEA_handler.c
 *
 *  Created on: May 24, 2024
 *      Author: karl.yamashita
 */


#include "main.h"

char NMEA_GNRMC_String[UART_DMA_QUEUE_DATA_SIZE] = "pending...";

GNRMC_t gnrmc_data = {0};


/*
 * Description: parse the GNRMC data and save to NMEA_String. We have commands to retrieve this string
 * 				or retrieve specific arguments.
 */
int NMEA_GNRMC_Data(char *msg)
{
	if(NMEA_CalculateChecksum(msg))
	{
		sprintf(NMEA_GNRMC_String, msg);

		sprintf(gnrmc_data.nmea, "%s", strtok_f(msg, ','));
		sprintf(gnrmc_data.time, "%s", strtok_f(NULL, ','));
		sprintf(gnrmc_data.status, "%s", strtok_f(NULL, ','));
		sprintf(gnrmc_data.lat, "%s", strtok_f(NULL, ','));
		sprintf(gnrmc_data.ns, "%s", strtok_f(NULL, ','));
		sprintf(gnrmc_data.lon, "%s", strtok_f(NULL, ','));
		sprintf(gnrmc_data.ew, "%s", strtok_f(NULL, ','));
		sprintf(gnrmc_data.spd, "%s", strtok_f(NULL, ','));
		sprintf(gnrmc_data.cog, "%s", strtok_f(NULL, ','));
		sprintf(gnrmc_data.date, "%s", strtok_f(NULL, ','));
#ifdef AD_4_10
		sprintf(gnrmc_data.mv, "%s", strtok_f(NULL, ','));
#endif
		sprintf(gnrmc_data.mvew, "%s", strtok_f(NULL, ','));
		sprintf(gnrmc_data.posMode, "%s", strtok_f(NULL, ','));
		sprintf(gnrmc_data.navStatus, "%s", strtok_f(NULL, ','));
	}

	return 0;
}

/*
 * Description: Parse command
 */
int NMEA_GNRMC_Command(char *msg, char *retStr)
{
	int status = NO_ERROR;
	char *token;
	char *rest = msg;
	char *result;
	char delim[] = ": ,|\r";

	result = strstr(rest, "?");
	if(result != NULL)
	{
		sprintf(retStr, NMEA_GNRMC_String); // return whole string
	}
	else // return specific argument
	{
		token = strtok_r(rest, delim, &rest);
		if(strncmp(token, "status", strlen("status")) == 0)
		{
			sprintf(retStr, gnrmc_data.status);
		}
		else if(strncmp(token, "time", strlen("time")) == 0)
		{
			sprintf(retStr, gnrmc_data.time);
		}
		else if(strncmp(token, "lat", strlen("lat")) == 0)
		{
			sprintf(retStr, gnrmc_data.lat);
		}
		else if(strncmp(token, "lon", strlen("lon")) == 0)
		{
			sprintf(retStr, gnrmc_data.lon);
		}
		else if(strncmp(token, "date", strlen("date")) == 0)
		{
			sprintf(retStr, gnrmc_data.date);
		}
		else if(strncmp(token, "spd", strlen("spd")) == 0)
		{
			sprintf(retStr, gnrmc_data.spd);
		}
		else
		{
			status = COMMAND_UNKNOWN;
		}
	}

	return status;
}

/*
 * Description: Calculates checksum of NMEA message between '$' and '*'.
 * Input: The NMEA message string
 * return: true if checksum matches, else false
 */
bool NMEA_CalculateChecksum(char *msg)
{
    uint8_t cksum = 0;
    uint8_t endCksum;
    int i = 0;
    char *_crc;

    for(i = 1; i < strlen(msg) - 5; i++) // minus the "*hh\r\n" characters
    {
        cksum ^= (uint8_t)msg[i];
    }

    _crc = strtok(&msg[++i], "\r"); // skip '*' character and save up to CR
    endCksum = strtol(_crc, NULL, 16); // save hex string as uint8_t value

    if(cksum == endCksum)
    {
        return true;
    }

    return false;
}

/*
* These two functions below was contributed by another user who figured out that the standard strtok_r 
*	would not work with consecutive commas in a string correctly. I do not take credit for these two functions.
*/
char *strtok_fr (char *s, char delim, char **save_ptr)
{
    char *tail;
    char c;

    if (s == NULL)
    {
        s = *save_ptr;
    }
    tail = s;
    if ((c = *tail) == '\0')
    {
        s = NULL;
    }
    else
    {
		do {
			if (c == delim)
			{
				*tail++ = '\0';
				break;
			}
		} while ((c = *++tail) != '\0');
	}
    *save_ptr = tail;
    return s;
}

char *strtok_f (char *s, char delim)
{
    static char *save_ptr;

    return strtok_fr (s, delim, &save_ptr);
}
