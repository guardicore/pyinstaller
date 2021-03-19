#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "old_machine_common_functions.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

char* getRequestDataJson(struct requestData reqData, char* requestFormat, char* systemStr) {
    char* tunnel;
    if (reqData.tunnel != NULL) {
        size_t tunnelStringSize = strlen(reqData.tunnel) + (2 * strlen("\"")) + 2;
        tunnel = (char *)malloc(tunnelStringSize);
        if (NULL == tunnel) {
            error("Malloc failed!");
        }
        snprintf(tunnel, tunnelStringSize, "%s%s%s", "\"", reqData.tunnel, "\"");
    } else {
        tunnel = (char *)malloc(sizeof("false"));
        if (NULL == tunnel) {
            error("Malloc failed!");
        }
        strcpy(tunnel, "false");
    }

    size_t responseSize = strlen(requestFormat) + strlen(reqData.osVersion) + strlen(reqData.hostname) +
                          strlen(tunnel) + strlen(reqData.IPstring) + strlen(systemStr);

    if (reqData.glibcVersion != NULL) {
        responseSize += strlen(reqData.glibcVersion);
    }

    // Concatenate into string for post data
    char* buf = calloc(responseSize, sizeof(char));
    if (NULL == buf) {
        free(tunnel);
        error("Malloc failed!");
    }
    if (reqData.glibcVersion != NULL) {
        snprintf(buf,
            responseSize,
            requestFormat,
            systemStr,
            reqData.osVersion,
            reqData.glibcVersion,
            reqData.hostname,
            tunnel,
            reqData.IPstring);
    } else {
        snprintf(buf,
            responseSize,
            requestFormat,
            systemStr,
            reqData.osVersion,
            reqData.hostname,
            tunnel,
            reqData.IPstring);
    }
    return buf;
}

// Concatenates a 2d char array of "size" using "joint" into a single string
char* concatenate(int size, char** array, const char* joint) {
    size_t jlen = strlen(joint);
    size_t* lens = malloc(size * sizeof(size_t));
    if (NULL == lens) {
        error("Malloc failed!");
    }
    size_t i;
    size_t total_size = (size-1) * (jlen) + 1;
    char *result, *p;
    for (i = 0; i < size; ++i) {
        lens[i] = strlen(array[i]);
        total_size += lens[i];
    }
    p = result = malloc(total_size);
    if (NULL == p) {
        free(lens);
        error("Malloc failed!");
    }
    for (i = 0; i < size; ++i) {
        memcpy(p, array[i], lens[i]);
        p += lens[i];
        if (i < (size-1)) {
            memcpy(p, joint, jlen);
            p += jlen;
        }
    }
    *p = '\0';
    return result;
}

void addPortToServer(char* server, char* new_port, size_t new_port_strlen) {
    strcat(server, ":");
    strncat(server, new_port, new_port_strlen);
}

void stripPort(char* old_port) {
    old_port[0] = '\0';
}

void replacePort(char* old_port, char* new_port, size_t new_port_strlen) {
    strncpy(old_port + 1, new_port, new_port_strlen);
    old_port[new_port_strlen + 1] = '\0';
}

// Replaces the port number in a server string.
//
// Example 1: replaceServerPort("10.0.0.43:5000", NULL) == "10.0.0.43"
// Example 2: replaceServerPort("10.0.0.43", NULL) == "10.0.0.43"
// Example 3: replaceServerPort("10.0.0.43:5000", "5001") ==  "10.0.0.43:5001"
// Example 4: replaceServerPort("10.0.0.43:50001", "6") ==  "10.0.0.43:6"
// Example 5: replaceServerPort("10.0.0.43", "5") ==  "10.0.0.43:5"
char* replaceServerPort(char* server, char* new_port) {
    size_t server_strlen = strlen(server);
    size_t new_port_strlen = new_port == NULL? 0 : strlen(new_port);
    size_t result_strlen = server_strlen + new_port_strlen + 1;

    char *result_string = (char*)malloc(sizeof(char) * (result_strlen + 1));
    if (result_string == NULL) {
        error("Memory allocation failed\n");
    }

    strncpy(result_string, server, server_strlen);
    result_string[server_strlen] = '\0';

    char * old_port = strstr(result_string, ":");
    if (old_port == NULL && new_port_strlen == 0) {
        return result_string;
    }

    if (old_port == NULL) {
        addPortToServer(result_string, new_port, new_port_strlen);
    } else if (new_port_strlen == 0) {
        stripPort(old_port);
    } else {
        replacePort(old_port, new_port, new_port_strlen);
    }

    return result_string;
}

int parseFlags(int argc, char * argv[], int* server_i, int* tunnel_i) {
    *tunnel_i = 0;
    *server_i = 0;
    int i;
    int monkey_or_dropper = 0;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tunnel") == 0) {
            *tunnel_i = i+1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--server") == 0) {
            *server_i = i+1;
        }
        if (strcmp(argv[i], "m0nk3y") == 0 || strcmp(argv[i], "dr0pp3r") == 0) {
         monkey_or_dropper = 1;
        }
    }
    if (!monkey_or_dropper) {
        printf("Missing monkey or dropper flag\n");
        return 1;
    } else if (server_i == 0) {
        printf("Missing server flag\n");
        return 1;
    }
    return 0;
}
