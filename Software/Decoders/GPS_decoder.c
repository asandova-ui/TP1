//"Written" and "designed" by TP1 Group 4 Telecomms Master UVa 2025-2026

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "minmea.h" //Library to decode NMEA0183

#define MINMEA_MAX_SENTENCE_LENGTH 256

#define INDENT_SPACES "  "

int main(void)
{
    char line[MINMEA_MAX_SENTENCE_LENGTH];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        printf("%s", line);
        switch (minmea_sentence_id(line, false)) {
            case MINMEA_SENTENCE_GLL: {
				struct minmea_sentence_gll frame;
				if(minmea_parse_gll(&frame, line)){
					printf(INDENT_SPACES "$xxGLL Latitude: %d \n  Longitude: %d \n UTC time: %d \n Status: %d\n",frame.latitude, frame.longitude,frame.time, frame.char
				}
				else {
                    printf(INDENT_SPACES "$xxGST sentence is not parsed\n");
                }
			} break;


			case MINMEA_INVALID: {
                printf(INDENT_SPACES "$xxxxx sentence is not valid\n");
            } break;

            default: {
                printf(INDENT_SPACES "$xxxxx sentence is not parsed\n");
            } break;
		}
	}
	return 0;
}
