#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include "assmbler.h"


static _ic = 0;
static _dc = 0;
static symbol_table[TAB_SIZE];
static word word_image[TAB_SIZE];
static instruction_word instruction_image[TAB_SIZE];
static int valid_file;


int is_label_decleration(const char *token);

int get_commend(const char *token);

void assemble(FILE *file)
{
    first_pass(file);
    /*second_pass(file);*/
}

int first_pass(FILE *file) {

    /*add method to restart static variables */
    valid_file = 1;
    /*------------*/
    int data_type;
    char *token, line[MAX_LINE_LEN];
    char *parameters;
    char *label;
    int symbol_type;
    int is_symbol;/*flag for defining symbol state ==OUT/ IN*/

    while (fgets(line,sizeof(line),file))
    {
        token = strtok(line, " \t\n");
        if((label = get_label(token)) != NULL)
        {/*if the line starts with a label*/
            is_symbol = TRUE;
            token = strtok(NULL, " \t\n");
        }
        if((data_type = get_data_type(token)) != NOT_DATA_TYPE)
        {
            symbol_type = DATA_TYPE;
            parameters = strtok(NULL, "\n");

            switch (data_type) {
                case (DATA):
                    get_data(parameters);
                    break;
                case (STRING):
                    get_string(parameters);
                    break;
                case (ENTRY):
                    break;
            }
        }
    }
}

int is_label_decleration(const char *token) {
    return ((strlen(token) >= 2) && token[strlen(token) - 1] == ':');
}

char *  get_label(const char *token) {

    char *label, *label_ptr;
    if(!is_label_decleration(token))
        return 0;

    label = strdup(token);
    label[strlen(label) -1 ] = '\0';

    if(get_commend(label) != NOT_OPCODE)
    {/*checks if the label is a commend name*/
        valid_file = NOT_VALID;
        printf("ERROR: the label %s with invalid name at line %s, can't use command keyword as label\n");
        return NULL;
    }
    if(!isalpha(*label))
    {
        valid_file = NOT_VALID;
        printf("ERROR: the label %s with invalid name at line %s, can't start label with non letter character\n");
        return NULL;
    }
    label_ptr = label;
    while(isalnum(*++label_ptr));/*skips all the chers of the label*/
    if(*label_ptr != '\0')
    {
        valid_file = NOT_VALID;
        printf("ERROR: the label %s with invalid name at line %s, can't have label with non letter or digit character\n");
        return NULL;
    }
    return label;
}

int get_commend(const char *token) {
    static const char *commend_list[] = {

            "mov", "cmp", "add",
            "sub", "not", "clr",
            "lea", "inc", "dec",
            "jmp", "bne", "red",
            "prn", "jsr", "rts",
            "stop"
    };

    int i;

    for (i = 0; i < sizeof(commend_list) / sizeof(commend_list[0]); i++)
        if (strcmp(token, commend_list[i]) == 0)
            return i;

    return NOT_OPCODE;

    return 0;
}

void get_string(const char *parameters) {

    if(is_string(parameters))

        while(isprint(*++parameters) && *parameters != '\"')
        {
            word_image[(_dc)++].word = *parameters;
        }
}
/**
 * returns non zero value if the string starts and ends with ""
 * */
int is_string(char *parameters) {
    return ((strlen(parameters) >= 3) &&
    (parameters[strlen(parameters) - 1] == '\"') &&
    (parameters[0] == '\"'));
}

void get_data(const char *parameters) {

/*@todo fix problem if ends with ,*/
    char *token = strtok(parameters, ",");
    skip_white_space(token);
    while (token != NULL) {
        int num;
        if (sscanf(token, "%d %d", &num) == 1) {
            word_image[_dc++].word = num;
        } else {
            printf("Invalid number\n");
            return;
        }
        token = strtok(NULL, ",");
    }
}

void skip_white_space(char *parameters) {
    while(isspace(*parameters))
        parameters++;
}

int get_data_type(const char *token) {
    int i;
    static char *data_keywords[] = {".data", ".string", "extern", "entry"};

    for(i = 0; i < 4; i++)
        if(!strcmp(data_keywords[i], token))
            return i;
    return NOT_DATA_TYPE;
}
