/**
* @file common.c
*  
* @brief Functions common for the whole project.
* 
* @date 25-Feb-2020
* @copyright GNU General Public License v3
* 
*/

#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

#include "mqtt_userdefs.h"


int process_arguments(int argc, char *argv[], start_arg_t *start_arg)
{
    int opt;

    snprintf(start_arg->location, sizeof(start_arg->location), "%s_%d", "location", getpid());

    while((opt = getopt(argc, argv, "b:p:l:")) != -1)
    {
        switch (opt)
        {
        case 'b':
            snprintf(start_arg->broker_hostname, sizeof(start_arg->broker_hostname), "%s", optarg);
            break;
        case 'p':
            start_arg->broker_port = (uint16_t) atoi(optarg);
            break;
        case 'l':
            snprintf(start_arg->location, sizeof(start_arg->location), "%s", optarg);
            break;
        default:
            break;
        }
    }

    return 0;
}
