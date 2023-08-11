#include <string.h>
#include <ctype.h>
#include "assmbler.h"


static ic = 0;
static dc = 0;
static struct nlist *symbol_table[TAB_SIZE];
static word word_image[TAB_SIZE];
static instruction_word instruction_image[TAB_SIZE];
static int valid_file;
static line_count;
static FILE *input_file;




void assemble(FILE *file)
{
    input_file = file;
    first_pass();
    /*second_pass();*/
}



int first_pass() {

    /*add method to restart static variables */
    valid_file = 1;
    line_count = 1;
    /*------------*/
    int data_type, command_type;
    char *token, line[MAX_LINE_LEN];
    char *data_values, param_1, param_2;
    symbol temp_symbol;/*temporary symbol struct that holds values for a symbol entry later*/
    int is_symbol;/*flag for defining symbol state ==OUT/ IN*/

    while (fgets(line,sizeof(line),input_file))
    {
        token = strtok(line, " \t\n");
        if((temp_symbol.label = get_label(token)) != NULL)
        {/*if the line starts with a label*/
            is_symbol = TRUE;
            token = strtok(NULL, " \t\n");
        }
        if((data_type = get_data_type(token)) != NOT_DATA_TYPE)
        {
            if(is_symbol == TRUE)
            {/*found all the information for making symbol entry*/
                temp_symbol.symbol_type = DATA_TYPE;
                temp_symbol.memory_address = dc;
                if (symbol_lookup(temp_symbol.label) != NULL) {/*there is already a as symbol with that label*/
                    valid_file = NOT_VALID;
                    printf("ERROR: the symbol \"%s\" is defined more then once at line number %d\n", temp_symbol.label, line_count);
                }
                if(symbol_install(&temp_symbol) == NULL){
                    valid_file = NOT_VALID;
                    printf("ERROR: failed to install the symbol \"%s\" at line number %d\n", temp_symbol.label, line_count);
                }
            }

            data_values = strtok(NULL, "\n");/*takes the rest of the line a parameter for the data type*/

            switch (data_type) {
                case (DATA):
                    get_data(data_values);
                    break;
                case (STRING):
                    get_string(data_values);
                    break;
                case (ENTRY):
                    break;/*@todo add here rest of the cases for entry and extern*/
            }
        }
        else if((command_type = get_commend(token)) != NOT_OPCODE)
        {/*if the token is a command*/
            temp_symbol.symbol_type=  COMMEND_TYPE;
            temp_symbol.memory_address = ic;
            token = strtok(NULL, "\n");/*takes the rest of the line as a parameter for the data type*/
                get_command_parameters(command_type, token);
        }
    line_count++;
    is_symbol = FALSE;/*rest the symbol reading flag*/
    }

   return valid_file;
}



int is_label_decleration(const char *token) {
    return ((strlen(token) >= 2) && token[strlen(token) - 1] == ':');
}
/***
 * returns a copy of the label if it is valid, returns null otherwise
 * */
char *get_label(const char *token) {

    char *label, *label_ptr;
    if(!is_label_decleration(token))
        return 0;

    label = strdup(token);
    label[strlen(label) -1 ] = '\0';

    if(get_commend(label) != NOT_OPCODE)
    {/*checks if the label is a commend name*/
        valid_file = NOT_VALID;
        printf("ERROR: the label %s with invalid name at line %d, can't use command keyword as label\n", label, line_count);
        return NULL;
    }
    if(!isalpha(*label))
    {
        valid_file = NOT_VALID;
        printf("ERROR: the label %s with invalid name at line %d, can't start label with non letter character\n", label, line_count);
        return NULL;
    }
    label_ptr = label;
    while(isalnum(*++label_ptr));
    if(*label_ptr != '\0')
    {
        valid_file = NOT_VALID;
        printf("ERROR: the label %s with invalid name at line %d, can't have label with non letter or digit character\n", label, line_count);
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
}

void get_string(const char *data_values) {

    if(is_string(data_values))
    {
        while (isprint(*++data_values) && *data_values != '\"')
            word_image[(dc)++].word = *(unsigned char *)data_values;
        if(*data_values != '\"')
        {
            valid_file = NOT_VALID;
            printf("ERROR: the character: %s is not valid for string a argument. at line %d", *data_values, line_count);
        }
        word_image[(dc)++].word = '\0'; /*appends the values of the string end of string indicator*/


    }
    else
    {
        valid_file = NOT_VALID;
        printf("ERROR: the string argument: %s is not with correct string format: \"<string>\" at line %d", data_values, line_count);
    }
}
/**
 * returns non zero value if the string starts and ends with ""
 * */
int is_string(char *data_values) {
    return ((strlen(data_values) >= 3) &&
    (data_values[strlen(data_values) - 1] == '\"') &&
    (data_values[0] == '\"'));
}

void get_data(const char *data_values) {
    char * s;/*just a holder*/
/*@todo fix problem if ends with ,*/
    char *token = strtok(data_values, ",");
    skip_white_space(token);
    while (token != NULL) {
        int num;
        if (sscanf(token, "%d %d", &num, s) == 1) {
            word_image[dc++].word = num;
        } else {
            printf("Invalid number\n");
            return;
        }
        token = strtok(NULL, ",");
    }
}

void skip_white_space(char *data_values) {
    while(isspace(*data_values))
        data_values++;
}

int get_data_type(const char *token) {
    int i;
    static char *data_keywords[] = {".data", ".string", "extern", "entry"};

    for(i = 0; i < 4; i++)
        if(!strcmp(data_keywords[i], token))
            return i;
    return NOT_DATA_TYPE;
}

struct nlist *symbol_lookup(char *label){
    return lookup(symbol_table, label, TAB_SIZE);
}
struct nlist *symbol_install(symbol *symbol_entry){
    return install(symbol_table, symbol_entry->label, symbol_entry, TAB_SIZE);

}
/**
 * Gets from the token the parameters of the command and checks if they are a valid arguments for that command,
 * prints relevant error message if not,
 * and stores the instruction word of the command in the instruction image for later use in the machine code.
 * */
void get_command_parameters(int command_type, char *token) {

}