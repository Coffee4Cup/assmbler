//
// Created by itay8 on 10/08/2023.
//

#ifndef ASSMBLER_ASSMBLER_H
#define ASSMBLER_ASSMBLER_H
#define TAB_SIZE 101
#define MAX_LINE_LEN 80
#define MAX_LABEL_LEN 31
#include <stdio.h>

typedef struct {
    char label[MAX_LABEL_LEN];
    int memory_address;
    int symbol_type
}symbol;

typedef struct
{
 unsigned word : 12;
}word;
typedef struct
{
    unsigned ARE : 2;
    unsigned opcode : 4;
    unsigned dest_operand :3;
    unsigned src_operand :3;
}instruction_word;

/*----------enums------------------*/
enum data_types{DATA, STRING, EXTERN, ENTRY, NOT_DATA_TYPE};
enum commend_types{
    MOV, CMP, ADD, SUB, NOT, CLR,
    LEA, INC, DEC, JMP, BNE, RED,
    PRN, JSR, RTS, STOP, NOT_OPCODE
};
enum {VALID, NOT_VALID};
enum {TRUE, FALSE};
enum symbol_type {DATA_TYPE, COMMEND_TYPE};
/*-------- main methods ------------*/
void assemble(FILE *file);
int first_pass(FILE *file);
int second_pass(FILE *file);

/*---------parsing methods-----------*/
int is_label_decleration(const char *token);
char * get_label(const char *token);
int get_data_type(const char *token);
void get_data(const char *parameters);
void skip_white_space(char *parameters);
void get_string(const char *parameters);
int is_string(char *parameters);

#endif //ASSMBLER_ASSMBLER_H
