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

#define SKIP_WHITE_SPACE(A) while(*(A) == ' ' || *(A) == '\t') (A)++
/*copy a word and changes the last char to be end of line char, for the cases when <word><,>, this copy only <word>*/
#define PUT_WORD(A,B) while((*(A)++ = *(B)++) != '\t' && *((A) - 1) != ',' && *((A) - 1) != ' '); \
*(A - 1) ='\0';

/*int main() {
    char *s = "                              parm1                                   ,  parm2                       \n";
    char *sf1 = (char *) malloc(9),*sf2 = (char *) malloc(9);
    char *sf1_p = sf1, *sf2_p = sf2;
    SKIP_WHITE_SPACE(s);
    PUT_WORD(sf1_p, s)
    SKIP_WHITE_SPACE(s);
    if(*s == ',')
    {
        s++;
        SKIP_WHITE_SPACE(s);
        PUT_WORD(sf2_p, s);
        SKIP_WHITE_SPACE(s);
    }

    printf("%s %s",sf1, sf2);


    return 0;
}*/



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

    symbol temp_symbol;/*temporary symbol struct that holds values for a symbol entry later*/
    int is_symbol;/*flag for defining symbol state ==OUT/ IN*/

    while (fgets(line,sizeof(line),input_file))
    {
        token = strtok(line, " \t\n");
        if(is_label_decleration(token))
        {/*if the line starts with a label*/
            temp_symbol.label = get_label(token);
             is_symbol = TRUE;
             token = strtok(NULL, " \t\n");
        }
        if((data_type = get_data_type(token)) != NOT_DATA_TYPE)
        {
            if(is_symbol == TRUE)
            {/*found all the information for making symbol entry*/
                temp_symbol.symbol_type = DATA_TYPE;
                temp_symbol.memory_address = dc;
                if (symbol_lookup(temp_symbol.label) != NULL)
                {/*there is already a as symbol with that label*/
                    valid_file = NOT_VALID;
                    printf("ERROR: the symbol \"%s\" is defined more then once at line number %d\n", temp_symbol.label, line_count);
                }
                if(symbol_install(&temp_symbol) == NULL)
                {
                    valid_file = NOT_VALID;
                    printf("ERROR: failed to install the symbol \"%s\" at line number %d\n", temp_symbol.label, line_count);
                }
                    is_symbol = FALSE;
            }

            token = strtok(NULL, "\n");/*takes the rest of the line a parameter for the data type*/

            switch (data_type) {
                case (DATA):
                    get_data(token);
                    break;
                case (STRING):
                    get_string(token);
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
        is_symbol = FALSE;/*reset the symbol's reading flag*/
        }

    return valid_file;
}


int is_label(const char *token){
    char *label_ptr;


    if (get_commend(token) != NOT_OPCODE || !isalpha(*token))
        return FALSE;

    label_ptr = token;
    while (isalnum(*++label_ptr));

    if (*label_ptr != '\0')
        return FALSE;


    return TRUE;
}

int is_label_decleration(const char *token) {
    return ((strlen(token) >= 2) && token[strlen(token) - 1] == ':');
}
/***
 * Return a copy of the label if it is valid, returns null otherwise.
 * Assumes that the label is without the ':'
 * */
char *get_label(const char *token) {

    char *label = (char*)malloc(90);
    char *label_ptr;


    label = strdup(token);
    if (label == NULL) {
        printf("ERROR: Memory allocation failed for label.\n");
        return NULL;
    }
    /*if it is a decleration, remove the ':' at the end of the label name*/
    if(is_label_decleration(token))
        {
        label[strlen(label) - 1] = '\0';
        }

        if (get_commend(label) != NOT_OPCODE)
            {/*checks if the label is a command name*/
            valid_file = NOT_VALID;
            printf("ERROR: the label %s with invalid name at line %d, can't use command keyword as label\n", label,
                   line_count);
            return NULL;
            }
        if (!isalpha(*label))
            {
            valid_file = NOT_VALID;
            printf("ERROR: the label %s with invalid name at line %d, can't start label with non letter character\n",
                   label, line_count);
            return NULL;
            }
        label_ptr = label;
        while (isalnum(*++label_ptr));
        if (*label_ptr != '\0')
            {
            valid_file = NOT_VALID;
            printf("ERROR: the label %s with invalid name at line %d, can't have label with non letter or digit character\n",
                   label, line_count);
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
/*gets a string that represents the value of the string data type and stores it in the image of declarations */
void get_string(const char *string_value) {

    if(is_string(string_value))
        {
        while (isprint(*++string_value) && *string_value != '\"')
            word_image[(dc)++].word = *(unsigned char *)string_value;
        if(*string_value != '\"')
            {
            valid_file = NOT_VALID;
            printf("ERROR: the character: %c is not valid for string a argument. at line %d\n", *string_value, line_count);
            }
        word_image[(dc)++].word = '\0'; /*appends "end-of-line" indicator*/


        }
    else
        {
        valid_file = NOT_VALID;
        printf("ERROR: the string argument: \"%s\" is not with correct string format: \"<string>\" at line %d", string_value, line_count);
        }
}
/**
 * returns non zero value if the string starts and ends with ""
 * */
int is_string(const char *data_values) {
    return ((strlen(data_values) >= 3) &&
            (data_values[strlen(data_values) - 1] == '\"') &&
            (data_values[0] == '\"'));
}



/* Function to parse and handle a single data value */
void get_data(const char *data_values) {
    int data_value;
    char *line_ptr, *end_ptr;

    line_ptr = (char *)data_values; /*Initialize line_ptr*/

   /* Loop through the data values*/
    while (*line_ptr) {
        SKIP_WHITE_SPACE(line_ptr);

        /*Convert the next data value*/
        data_value = strtol(line_ptr, &end_ptr, 10);

         /*Check if a valid number was parsed*/
        if (line_ptr == end_ptr) {
            /** No valid data value found **/
            printf("ERROR: No valid data value found at line %d.\n", line_count);
            valid_file = NOT_VALID;
            break;
            }

        /*Check for non-digit characters*/
        if (ispunct(*line_ptr) && *line_ptr != ',') {
            /** Wrong syntax for data values **/
            printf("ERROR: Wrong syntax for data values found at line %d.\n", line_count);
            valid_file = NOT_VALID;
            break;
            }

        /*Check if the integer can be stored in a 12-bit word*/
        if (!data_value_is_valid(data_value)) {
            /** Data value out of range **/
            printf("ERROR: Data value out of range at line %d.\n", line_count);
            valid_file = NOT_VALID;
            break;
            } else {
            /*Store the valid data value in the image*/
            put_data_in_image(data_value);
            }

       /* Move to the next character after the parsed number*/
        line_ptr = end_ptr;

        /*Check for a comma and prepare for the next iteration*/
        if (*line_ptr == ',') {
            line_ptr++;  /*Move past the comma*/
            } else if (*line_ptr) {
            /** Unexpected character found **/
            printf("ERROR: Unexpected character found at line %d.\n", line_count);
            valid_file = NOT_VALID;
            break;
            }
        }
}


/**
 * check is the value of the int can be expressed within one memory word of 12 bits
 * */
int data_value_is_valid(int value)
{
    if(value <= MAX_DATA_VALUE && value >= MIN_DATA_VALUE)
        return TRUE;
    return FALSE;
}

void put_data_in_image(int value)
{
    if (dc + 1 <= TAB_SIZE - 1)/*if there is space in memory for the data value*/
    word_image[dc++].word = value;
    else
        {
        printf("ERROR: no more space in assembler's data image in line: %d", line_count);
        valid_file = NOT_VALID;
        }
}


int get_data_type(const char *token)
{
    int i;
    static char *data_keywords[] = {".data", ".string", "extern", "entry"};

    if(*token == '.')
        {/*starts with a point to mark it as a data type*/
        for (i = 0; i < 4; i++)
            if (!strcmp(data_keywords[i], token))
                return i;

        valid_file = NOT_VALID;/*there was a data type point but the data type is not defined*/
        printf("ERROR: unknown data type at line %d", line_count);
        }
    return NOT_DATA_TYPE;
}

struct nlist *symbol_lookup(char *label)
        {
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
void get_command_parameters(int command_type,const char *parameters) {

    char *param1= (char *)malloc(MAX_LINE_LEN * sizeof(char)),
         *param2= (char *)malloc(MAX_LINE_LEN * sizeof(char));
    int param1_type = 0, param2_type = 0;/*holds the types of operands given to the function, can be 1, 3, 5 or 0 (using the parameter_types enum) if the given param_type is not recognized */
    char *param_temp_ptr;
    char *end_ptr;/*end_ptr for checking the function strtol() returned value*/

    SKIP_WHITE_SPACE(parameters);
    param_temp_ptr = param1;
    PUT_WORD(param1, parameters);
    param1= param_temp_ptr;
    /*gets the operand type into the &para1_type variable*/
    get_operand_type(param1, &param1_type);

    SKIP_WHITE_SPACE(parameters);
    if(*parameters++ == ',')
    {
        SKIP_WHITE_SPACE(parameters);
        param_temp_ptr = param2;
        PUT_WORD(param2, parameters);
        param2 = param_temp_ptr;
        get_operand_type(param2, &param2_type);

    }

    /*check if the parameters given are matching the function's prototype*/
    is_prototype_match(command_type, param1_type, param2_type);


}

void get_operand_type(const char *operand,  int *operand_type) {
    if(is_string(operand) || is_data(operand))
        *operand_type = IMMEDIATE_ADD;
    else if(is_label(operand))/*might be a label*/
        *operand_type = DIR_ADD;
    else if(is_register(operand))
        *operand_type = DIR_REGISTER_ADD;

    else
        {/*if the operand type is not known*/
        *operand_type = NOT_ADD;
        printf("an operand at line %d is not known operand type/ not valid", line_count);
        valid_file = NOT_VALID;
        }


}
/*returns true if the operand starts with the @, decleration use of a register*/
int is_register(const char *operand) {
    return (operand != NULL && *operand == '@');
}
/*checks if the given string is a valid data type*/
int is_data(const char *data) {

    int num;
    char * data_ptr;
    num = (int) strtol(data, &data_ptr, 10);
    return (*data_ptr == '\0' && data_value_is_valid(num));

}

void is_prototype_match(int command_type, int param1_type, int param2_type)
{

    const static prototype prototypes[16] = {
            { {TRUE, TRUE, TRUE}, {FALSE, TRUE, TRUE} },  /* mov */
        { {TRUE, TRUE, TRUE}, {TRUE, TRUE, TRUE} },   /* cmp */
        { {TRUE, TRUE, TRUE}, {FALSE, TRUE, TRUE} },  /* add */
        { {TRUE, TRUE, TRUE}, {FALSE, TRUE, TRUE} },  /* sub */
        { {FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE} },  /* not */
        { {FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE} },  /* clr */
        { {FALSE, TRUE, FALSE}, {FALSE, TRUE, TRUE} }, /* lea */
        { {FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE} },  /* inc */
        { {FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE} },  /* dec */
        { {FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE} },  /* jmp */
        { {FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE} },  /* bne */
        { {FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE} },  /* red */
        { {FALSE, FALSE, FALSE}, {TRUE, TRUE, FALSE} }, /* prn */
        { {FALSE, FALSE, FALSE}, {FALSE, TRUE, TRUE} },  /* jsr */
        { {FALSE, FALSE, FALSE}, {FALSE, FALSE, FALSE} }, /* rts */
        { {FALSE, FALSE, FALSE}, {FALSE, FALSE, FALSE} }  /* stop */
    };
    if((MOV <= command_type && command_type <= SUB || command_type == LEA) && param1_type  && param2_type )/*if the command is a two-parameter type*/
    {
        is_operand_match(prototypes[command_type].src_operand, param1_type);
        is_operand_match(prototypes[command_type].dest_operand, param2_type);
    }
    else if((command_type == NOT || command_type == CLR || INC <= command_type && command_type <= JSR) &&  !param1_type && param2_type)
    {
        is_operand_match(prototypes[command_type].dest_operand, param1_type);
    }
    else if(!((command_type == RTS || command_type == STOP) &&  !param1_type && !param2_type))
    {/*if the */
        printf("ERROR: The command doesn't match the number of parameters given in line %d", line_count);
        valid_file = NOT_VALID;
    }

}

void is_operand_match(operand_type operand_type, int type) {

    /*if there is a mismatch between the type of the operand and the type of parameter that the function can take in its place */
     if(!(type == IMMEDIATE_ADD && operand_type.immediate_address ||
        type == DIR_ADD && operand_type.Direct_address ||
        type == DIR_REGISTER_ADD && operand_type.dir_register_address))
         {
         printf("ERROR: mismatch between parameter type accepted and operand type at line %d ", line_count);
         }
}







