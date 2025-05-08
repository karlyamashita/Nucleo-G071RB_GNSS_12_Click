/*
 * NMEA_handler.c
 *
 *  Created on: May 24, 2024
 *      Author: karl.yamashita
 */


#include "main.h"

char NMEA_GNRMC_String[UART_DMA_QUEUE_DATA_SIZE] = "pending...";

/*
 * Description: parse the GNSS data and save to NMEA_String. We have commands to retrieve this or partial arguments.
 */
int NMEA_GNRMC(char *msg)
{
	bool pass;

	pass = NMEA_CalculateChecksum(msg);
	if(pass)
	{
		char *nmea = strtok_f(msg, ',');
		char *time = strtok_f(NULL, ',');
		char *status = strtok_f(NULL, ',');
		char *lat = strtok_f(NULL, ',');
		char *ns = strtok_f(NULL, ',');
		char *lon = strtok_f(NULL, ',');
		char *ew = strtok_f(NULL, ',');
		char *spd = strtok_f(NULL, ',');
		char *cog = strtok_f(NULL, ',');
		char *date = strtok_f(NULL, ',');
		//char *mv = strtok_f(NULL, ','); // supported in AD 4.10 and later
		char *mvew = strtok_f(NULL, ',');
		char *posMode = strtok_f(NULL, ',');
		char *navStatus = strtok_f(NULL, '*');

		sprintf(NMEA_GNRMC_String, "%s = time: %s, status: %s, lat: %s, ns: %s, lon: %s, ew: %s, spd: %s, cog: %s, date: %s, mvew: %s, posMode: %s, navStatus: %s",
							nmea, time, status, lat, ns, lon, ew, spd, cog, date, mvew, posMode, navStatus);
	}

	return 0;
}

/*
 * Description: Parse command
 */
int NMEA_GNRMC_Parse(char *msg, char *retStr)
{
	int status = NO_ERROR;
	char *token; // 1 or 0
	char *rest = msg;
	char *result;
	char delim[] = ": ,|\r";
	uint32_t idx = 0;
	char copy[UART_DMA_QUEUE_DATA_SIZE] = {0};
	char *ptr = copy;
	char strToFind[16] = {0};

	memcpy(ptr, &NMEA_GNRMC_String, strlen(NMEA_GNRMC_String)); // copy current string as to

	result = strstr(rest, "?");
	if(result != NULL)
	{
		sprintf(retStr, NMEA_GNRMC_String); // return whole string
	}
	else // return specific argument
	{
		token = strtok_r(rest, delim, &rest); // get enable status
		if(strncmp(token, "time", strlen("time")) == 0)
		{
			sprintf(strToFind, "time:");
		}
		else if(strncmp(token, "lat", strlen("lat")) == 0)
		{
			sprintf(strToFind, "lat:");
		}
		else if(strncmp(token, "lon", strlen("lon")) == 0)
		{
			sprintf(strToFind, "lon:");
		}
		else if(strncmp(token, "date", strlen("date")) == 0)
		{
			sprintf(strToFind, "date:");
		}
		else
		{
			status = COMMAND_UNKNOWN;
		}

		if(status != NO_ERROR) return status;

		idx = FindWordIndex(ptr, strToFind);
		ptr += idx;
		idx = FindCharIndex(ptr, ',');
		snprintf(retStr, idx + 1, "%s", ptr);
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

int FindWordIndex(const char *msg, const char *word)
{
    const char *result = strstr(msg, word);
    if (result == NULL)
    {
        return -1;
    }
    else
    {
        return (int)(result - msg); // return index
    }
}

int FindCharIndex(const char *str, char c)
{
	if (str == NULL)
	{
		return -1;
	}

	for (int i = 0; str[i] != '\0'; i++)
	{
		if (str[i] == c)
		{
			return i;
		}
	}

	return -1;
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
