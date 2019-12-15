//
// Created by maxim on 26.11.19.
//

const int MAX_SINGLE_WORD_LENGTH = 256;
const int MAX_VARIABLE_LENGTH = 256;
const int MAX_FUNCTION_BUFFER = 8192;

enum NodeTypes {
    PROGRAMM_ROOT = 0,
    DECLARATION = 1,
    FUNCTION = 2,
    VARLIST = 3,
    ID = 4,
    BLOCK = 5,
    IF = 6,
    WHILE = 7,
    OP = 8,
    EXPRESSION = 9,
    VAR = 10,
    RETURN = 11,
    INPUT = 12,
    OUTPUT = 13,
    INITIALIZE = 14,
    ASSIGNMENT = 15,
    NUMBER = 16,
    OPERATOR = 17,
    CALL = 18,
    C = 19,
    ADD = 100,
    SUB = 101,
    MUL = 102,
    DIV = 103,
    SIN = 104,
    COS = 105,
    TAN = 106,
    SQR = 107,
    EQUAL = 200,
    ABOVE = 201
};