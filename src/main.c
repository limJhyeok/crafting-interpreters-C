#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h> // offsetof
#include <setjmp.h> // error handling

#define N_RESERVED_WORD 16
#define MAX_LEN_RESERVED_WORD 10
#define MAX_TOKEN_LEXEME_SIZE 20
#define MAX_TOKEN_LITERAL_SIZE 100
#define INITIAL_LIST_SIZE 2

jmp_buf run_error_jmp;


// Excpetion - start
typedef struct {
    int code;                 // 예외 코드
    char message[256];        // 예외 메시지
} Exception;
// Exception - end

Exception current_exception;

void throw_exception(int code, const char *message) {
    current_exception.code = code;           // 코드 설정
    strncpy(current_exception.message, message, sizeof(current_exception.message) - 1);
    current_exception.message[sizeof(current_exception.message) - 1] = '\0'; // null-terminate
    longjmp(run_error_jmp, code);            // 예외 발생
}

// Token - start

typedef enum TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    STAR, DOT, COMMA, PLUS, MINUS, SEMICOLON, SLASH,

    // One or two character tokens.
    EQUAL, EQUAL_EQUAL,
    BANG, BANG_EQUAL,
    LESS, LESS_EQUAL,
    GREATER, GREATER_EQUAL,

    // Literals.
    STRING, NUMBER, IDENTIFIER,

    // Keywords.
    AND, CLASS, ELSE, FALSE, FOR, FUN, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,
    
    END_OF_FILE,
    
    INVALID_TOKEN
} TokenType;

const char *TokenTypeStrs[] = {
    "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE",
    "STAR", "DOT", "COMMA", "PLUS", "MINUS", "SEMICOLON", "SLASH",
    
    "EQUAL", "EQUAL_EQUAL",
    "BANG", "BANG_EQUAL", 
    "LESS", "LESS_EQUAL", 
    "GREATER", "GREATER_EQUAL", 
    
    "STRING", "NUMBER", "IDENTIFIER", 
    
    "AND", "CLASS", "ELSE", "FALSE", "FOR", "FUN", "IF", "NIL", "OR",
    "PRINT", "RETURN", "SUPER", "THIS", "TRUE", "VAR", "WHILE",
    
    "EOF",
    
    "INVALID_TOKEN"
};


typedef struct Token {
    TokenType type;
    char *lexeme;
    char *literal;
    int line;
    struct Token *prev;
    struct Token *next;
} Token;


void initTokenList();
int isEmptyList();
int releaseTokenList();
Token* createToken(TokenType type, char *lexeme, char *literal, int line);
int insertAtTail(TokenType type, char *lexeme, char *literal, int line);
int getSize();
void printToken(const Token *token);
void printTokenList();

int copyStr(char *dest, char *src);

char *read_file_contents(const char *filename);

int scanning(const char *file_contents);

int isDigit(const char c);
int isAlpha(const char c);
int isAlphaNumeric(const char c);

int isIn(const char *str, const char c);

void trimTrailingZeros(char *str);

TokenType getReservedToken(const char *c);

// Token - end

// Object - start

typedef struct Object{
    TokenType type;
    void* value;
} Object;

typedef struct {
    double number;
} NumberValue;

typedef struct {
    int boolean; // 1 (true) or 0 (false)
} BoolValue;

// 문자열 값 저장용 구조체
typedef struct {
    char* string;
} StringValue;

Object* createObject(TokenType type, const char* literal);

// Object - end

// Expression - start
struct Visitor;

typedef struct Expr
{
    void* (*accept)(struct Expr *self, struct Visitor *visitor);
} Expr;

typedef struct Visitor {
    void *(*visitBinaryExpr)(struct Visitor *self, Expr *expr);
    void *(*visitUnaryExpr)(struct Visitor *self, Expr *expr);
    void *(*visitGroupingExpr)(struct Visitor *self, Expr *expr);
    void *(*visitLiteralExpr)(struct Visitor *self, Expr *expr);
} Visitor;

typedef struct ExprBinary
{
    Expr base;
    Expr *left;
    Token *operator;
    Expr *right;
} ExprBinary;

typedef struct ExprUnary
{
    Expr base;
    Token *operator;
    Expr *right;
} ExprUnary;

typedef struct ExprGrouping
{
    Expr base;
    Expr *expression;
} ExprGrouping;

typedef struct ExprLiteral
{
    Expr base;
    TokenType type;
    char *value;
} ExprLiteral;

void* ExprBinaryAccept(Expr *self, Visitor *visitor);
void* ExprUnaryAccept(Expr *self, Visitor *visitor);
void* ExprGroupingAccept(Expr *self, Visitor *visitor);
void* ExprLiteralAccept(Expr *self, Visitor *visitor);

typedef struct AstPrinter{
    Visitor base;
    char* (*print)(Visitor *self, Expr *expr);
} AstPrinter;

char *parenthesize(const char *name, char **exprs, int count);

void *AstPrinterVisitBinaryExpr(Visitor *self, Expr *expr);
void *AstPrinterVisitUnaryExpr(Visitor *self, Expr *expr);
void *AstPrinterVisitGroupingExpr(Visitor *self, Expr *expr);
void *AstPrinterVisitLiteralExpr(Visitor *self, Expr *expr);

char *print(Visitor *self, Expr *expr);
AstPrinter *newAstPrinter();

// Expression - end


// Statement - start
struct StmtVisitor;

typedef struct Stmt{
    void* (*accept)(struct Stmt *self, struct StmtVisitor *visitor);
} Stmt;

typedef struct Print {
    Stmt base;
    Expr* expression;
} Print;

typedef struct Expression {
    Stmt base;
    Expr* expression;
} Expression;

typedef struct StmtVisitor{
    void* (*visitExpressionStmt)(struct StmtVisitor* self, Stmt* stmt);
    void* (*visitPrintStmt)(struct StmtVisitor* self, Stmt* stmt);
} StmtVisitor;

// Statement - end

void* PrintStmtAccept(Stmt *self, StmtVisitor *stmt_visitor);
void* ExpressionStmtAccept(Stmt *self, StmtVisitor *stmt_visitor);

Print* createPrintStmt(Expr* expression);
Expression* createExpressionStmt(Expr* expressoin);

void* InterpreterVisitExpressionStmt(StmtVisitor *self, Stmt* stmt);
void* InterpreterVisitPrintStmt(StmtVisitor *self, Stmt* stmt);

typedef enum {
    PRINT_STMT,
    EXPRESSION_STMT
} ElementType;

typedef struct Element {
    ElementType type;
    union {
        Print* print_stmt;
        Expression* expr_stmt;
    } data;
} Element;

typedef struct Array {
    Element* elements;        // 실제 데이터를 저장하는 배열
    size_t count;        // 배열에 저장된 요소의 개수
    size_t capacity;    // 배열의 용량
} Array;


// TODO: Array에 서로 다른 element 저장하게 만들기

Array* createArray(size_t initialCapacity);
void addElement(Array* array, Element element); 
void* getElement(Array* array, size_t index);
void releaseArray(Array* array);

// Parser - start

typedef struct ParseError{
} ParseError;

ParseError* createParseError();

typedef struct Parser {
    Token* current;
    Token* (*peek)(struct Parser*);
    int (*isAtEnd)(struct Parser*);
    Token* (*previous)(struct Parser*);
    Token* (*advance)(struct Parser*);
    int (*check)(struct Parser*, TokenType);
    int (*match)(struct Parser*, TokenType*, size_t num_types);
    Expr* (*primary)(struct Parser*);
    Expr* (*unary)(struct Parser*);
    Expr* (*factor)(struct Parser*);
    Expr* (*term)(struct Parser*);
    Expr* (*comparison)(struct Parser*);
    Expr* (*equality)(struct Parser*);
    Expr* (*expression)(struct Parser*);
    Stmt* (*statement)(struct Parser*);
    Stmt* (*printStatement)(struct Parser*);
    Stmt* (*expressionStatement)(struct Parser*);
    ParseError* (*parserError)(struct Parser*, Token*, char*);
    void (*synchronize)(struct Parser*);
    Array* (*parse)(struct Parser*);
} Parser;

Token* peek(Parser* self);
int isAtEnd(Parser* self);
Token* previous(Parser* self);
Token* advance(Parser* self);
int check(Parser* self, TokenType type);
int match(Parser *self, TokenType* types, size_t num_types);

Expr* primary(Parser *self);
Expr* unary(Parser *self);
Expr* factor(Parser *self);
Expr* term(Parser* self);
Expr* comparison(Parser* self);
Expr* equality(Parser *self);
Expr* expression(Parser* self);
Stmt* statement(Parser* self);
Stmt* printStatement(Parser* self);
Stmt* expressionStatement(Parser* self);
ParseError* parserError(Parser* self,Token* token, char* message);
void *synchronize(Parser* self);
Array* parse(Parser* self);

Token* consume(Parser* self, TokenType type, char* message);

Parser* createParser();

// Parser - end

Token *g_head_pointer;
Token *g_tail_pointer;
int g_token_list_size = 0;

int had_error = 0;
void report(int line, char* where, char* message);
void error(Token* token, char* message);    

// Interpreter - start

typedef struct Interpreter {
    Visitor base;
    StmtVisitor stmt_visitor;
    Object* (*evaluate)(struct Interpreter* self, Expr* expr);
    void (*execute)(struct Interpreter* self, Stmt* stmt);
    void (*interpret)(struct Interpreter* self, Array* array);
} Interpreter;

typedef struct RuntimeError {
    Token token;
    char* message;
} RuntimeError;

RuntimeError* createRuntimeError(Token token, char* message);
RuntimeError* checkNumberOperand(Token operator, Object operand);
RuntimeError* checkNumberOperands(Token operator, Object left, Object right);
int runtime_error_flag = 0;

void* InterpreterVisitLiteralExpr(Visitor* self, Expr* expr);
void* InterpreterVisitGroupingExpr(Visitor* self, Expr* expr);
void* InterpreterVisitUnaryExpr(Visitor* self, Expr* expr);
void* InterpreterVisitBinaryExpr(Visitor* self, Expr* expr);
Object* evaluate(struct Interpreter* self, Expr* expr);
void execute(struct Interpreter* self, Stmt* stmt);
void interpret(struct Interpreter* self, Array* statements);
char* stringify(Object object);

int endsWith(char *c, size_t c_size, char *end, size_t end_size);

Interpreter *createInterpreter();

int isTruthy(Object* object);
TokenType isEqual(Object* a, Object* b);

Object* quotientOperation(Object* left, Object* right);
Object* multiplyOperation(Object* left, Object* right); 
Object* plusOperation(Object* left, Object* right);
Object* minusOperation(Object* left, Object* right);

int isGreater(double left, double right);
int isGreaterEqual(double left, double right);
int isLess(double left, double right);
int isLessEqual(double left, double right);
Object* relationalOperation(Object* left, Object* right, int (*comparison)(double, double));

// Interpreter - end

int main(int argc, char *argv[]) {
    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (argc < 3) {
        fprintf(stderr, "Usage: ./your_program tokenize <filename>\n");
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "tokenize") == 0) {
        fprintf(stderr, "Logs from your program will appear here!\n");
        
        char *file_contents = read_file_contents(argv[2]);
        if (strlen(file_contents) > 0) {
            int has_error = scanning(file_contents);
            
            printTokenList();
            releaseTokenList();

            if (has_error){
                exit(65);
            } else {
                exit(0);
            }

        }
        if (strlen(file_contents) == 0){
            printf("EOF  null\n");
        } 
        
        free(file_contents);
    } 
    else if (strcmp(command, "parse") == 0){
        char *file_contents = read_file_contents(argv[2]);
        if (strlen(file_contents) > 0) {
            int has_error = scanning(file_contents);

            if (has_error){
                releaseTokenList();
                exit(65);
            } 

            Parser* parser = createParser();

            Array* statements = parser->parse(parser);

            if (had_error){
                free(parser);
                releaseArray(statements);
                exit(65);
            }

            AstPrinter *printer = newAstPrinter();
            char *output;

            for (int i = 0; i < statements->count; i++){
                Expr* expr;
                Element* elem = getElement(statements, i);
                if (elem->type == PRINT_STMT){
                    expr = elem->data.print_stmt->expression;
                } else if (elem->type == EXPRESSION_STMT){
                    expr = elem->data.expr_stmt->expression;
                }
                output = printer->print((Visitor *)printer, expr);
                printf("%s\n", output);
            }


            free(printer);
            releaseArray(statements);
            free(parser);
            releaseTokenList();
        }

        free(file_contents);

    }
    else if (strcmp(command, "evaluate") == 0){
        char *file_contents = read_file_contents(argv[2]);
        if (strlen(file_contents) > 0) {
            int has_error = scanning(file_contents);

            if (has_error){
                releaseTokenList();
                exit(65);
            } 

            Parser* parser = createParser();

            Array* statements = parser->parse(parser);

            if (had_error){
                free(parser);
                releaseArray(statements);
                exit(65);
            }

            Interpreter* interpreter = createInterpreter();
            interpreter->interpret(interpreter, statements);

            free(parser);
            releaseArray(statements);
            free(interpreter);
            releaseTokenList();
        }

        free(file_contents);
    }
    else if (strcmp(command, "run") == 0){
        char *file_contents = read_file_contents(argv[2]);
        if (strlen(file_contents) > 0) {
            int has_error = scanning(file_contents);

            if (has_error){
                releaseTokenList();
                exit(65);
            } 

            Parser* parser = createParser();

            Array* statements = parser->parse(parser);

            if (had_error){
                free(parser);
                releaseArray(statements);
                exit(65);
            }

            Interpreter* interpreter = createInterpreter();
            interpreter->interpret(interpreter, statements);

            free(parser);
            releaseArray(statements);
            free(interpreter);
            releaseTokenList();
        }

        free(file_contents);
    }
    else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}

char *read_file_contents(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error reading file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *file_contents = malloc(file_size + 1);
    if (file_contents == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(file_contents, 1, file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Error reading file contents\n");
        free(file_contents);
        fclose(file);
        return NULL;
    }

    file_contents[file_size] = '\0';
    fclose(file);

    return file_contents;
}


int scanning(const char *file_contents){
    const char *now = &file_contents[0];
    size_t file_size = strlen(file_contents);

    initTokenList();
    TokenType now_type;
    char now_lexeme[MAX_TOKEN_LEXEME_SIZE] = "";
    char now_literal[MAX_TOKEN_LITERAL_SIZE] = "";
    int line = 1;

    int has_error = 0;

    for (int i = 0; i < file_size; i++){
        switch (*now) {
            case '(':
                insertAtTail(LEFT_PAREN, "(", "\0", line);
                break;
            case ')':
                insertAtTail(RIGHT_PAREN, ")", "\0", line);
                break;
            case '{':
                insertAtTail(LEFT_BRACE, "{", "\0", line);
                break;
            case '}':
                insertAtTail(RIGHT_BRACE, "}", "\0", line);
                break;
            case '*':
                insertAtTail(STAR, "*", "\0", line);
                break;
            case '.':
                insertAtTail(DOT, ".", "\0", line);
                break;
            case ',':
                insertAtTail(COMMA, ",", "\0", line);
                break;
            case '+':
                insertAtTail(PLUS, "+", "\0", line);
                break;
            case '-':
                insertAtTail(MINUS, "-", "\0", line);
                break;
            case '=':
                if (*(now + 1) == '='){
                    insertAtTail(EQUAL_EQUAL, "==", "\0", line);
                    now++;
                    i++;
                } else{
                    insertAtTail(EQUAL, "=", "\0", line);
                }
                break;
            case '!':
                if (*(now + 1) == '='){
                        insertAtTail(BANG_EQUAL, "!=", "\0", line);
                        now++;
                        i++;
                } else{
                    insertAtTail(BANG, "!", "\0", line);
                }
                break;            
            case '<':
                if (*(now + 1) == '='){
                        insertAtTail(LESS_EQUAL, "<=", "\0", line);
                        now++;
                        i++;
                } else{
                    insertAtTail(LESS, "<", "\0", line);
                }
                break; 
            case '>':
                if (*(now + 1) == '='){
                        insertAtTail(GREATER_EQUAL, ">=", "\0", line);
                        now++;
                        i++;
                } else{
                    insertAtTail(GREATER, ">", "\0", line);
                }
                break; 
            case '/':
                if (*(now + 1) == '/'){
                    while ((*(now) != '\n') && (i < file_size)){
                        now++;
                        i++;
                    }
                    if (*(now) == '\n') line++;
                    now++;
                    continue;
                } else{
                    insertAtTail(SLASH, "/", "\0", line);
                    break; 
                }
            case '\t':
                now++;
                continue;
            case ' ':
                now++;
                continue;
            case ';':
                insertAtTail(SEMICOLON, ";", "\0", line);
                break;
            case '\n':
                line++;
                now++;
                continue;
            case '"':
                int start = i;
                int end = -1;
                while ((*(now+1) != '"')){
                    if (i >= file_size){
                        has_error = 1;
                        fprintf(stderr, "[line %d] Error: Unterminated string.\n", line);
                        break;
                    }
                    now++;
                    i++;
                }
                if (has_error) continue;
                now++;
                i++;

                end = i;
                int lexeme_length = end - start + 1;
                now_type = STRING;

                strncpy(now_lexeme, file_contents + start, lexeme_length);
                now_lexeme[lexeme_length] = '\0';

                int literal_length = end - start - 1;
                strncpy(now_literal, file_contents + start + 1, literal_length);
                now_literal[literal_length] = '\0';

                insertAtTail(STRING, now_lexeme, now_literal, line);
                break;
            default:
                if (isDigit(*now)){
                    int start = i;
                    int end = -1;
                    while ((isDigit(*(now+1)) || *(now+1) == '.')){
                        now++;
                        i++;
                    }                    
                    end = i;

                    int lexeme_length = end - start + 1;
                    strncpy(now_lexeme, file_contents + start, lexeme_length);
                    now_lexeme[lexeme_length] = '\0';
                    copyStr(now_literal, "\0"); 

                    now_type = NUMBER;
                    if (isIn(now_lexeme, '.')){
                        strncpy(now_literal, file_contents + start, lexeme_length);
                        trimTrailingZeros(now_literal);
                    } else {
                        int literal_length = end - start + 1 + 2;
                        strncpy(now_literal, file_contents + start, literal_length);
                        now_literal[literal_length-2] = '.';
                        now_literal[literal_length-1] = '0';
                        now_literal[literal_length] = '\0';
                    }
                    insertAtTail(NUMBER, now_lexeme, now_literal, line);
                    break;
                } else if (isAlpha(*now)){
                    int start = i;
                    int end = -1;
                    while (isAlphaNumeric(*(now + 1))){
                        now++;
                        i++;
                    }
                    end = i;
                    int lexeme_length = end - start + 1;
                    strncpy(now_lexeme, file_contents + start, lexeme_length);
                    now_lexeme[lexeme_length] = '\0';
                    
                    if (getReservedToken(now_lexeme) != INVALID_TOKEN){
                        now_type = getReservedToken(now_lexeme); 
                    } else {
                        now_type = IDENTIFIER;
                    }
                    insertAtTail(now_type, now_lexeme, "\0", line);
                    break;
                }
                fprintf(stderr, "[line %d] Error: Unexpected character: %c\n", line, *now);
                now++;
                has_error = 1;
                continue; 

        }
        now++;
    }
    insertAtTail(END_OF_FILE, "\0", "\0", line);

    if (has_error){
        return 1;
    } 

    return 0;
}

void* InterpreterVisitLiteralExpr(Visitor* self, Expr* expr){
    ExprLiteral* expr_literal = (ExprLiteral*)expr;
    
    Object* object = createObject(expr_literal->type, expr_literal->value);
    return object;
}

void* InterpreterVisitGroupingExpr(Visitor* self, Expr* expr){
    ExprGrouping* expr_grouping = (ExprGrouping*)expr;

    return evaluate((Interpreter*)self, expr_grouping->expression);
}

Object* evaluate(struct Interpreter* self, Expr* expr){
    return (Object*)expr->accept(expr, &self->base);
}

void execute(Interpreter* self, Stmt* stmt){
    stmt->accept(stmt, &self->stmt_visitor);
}

void* InterpreterVisitUnaryExpr(Visitor* self, Expr* expr){
    ExprUnary* expr_unary = (ExprUnary*)expr;

    Object* right = evaluate((Interpreter*)self, expr_unary->right);
    switch (expr_unary->operator->type){
        case MINUS:
            RuntimeError* runtime_error = checkNumberOperand(*expr_unary->operator, *right);
            if (runtime_error_flag) return runtime_error;
            double number = (((NumberValue*)right->value)->number);
            number = -number;
            ((NumberValue*)right->value)->number = number;
            return right;
        case BANG:
            int is_truthy = isTruthy(right);
            if (is_truthy){
                return createObject(FALSE, "false");
            }
            return createObject(TRUE, "true");
    }

    return NULL;
}

RuntimeError* checkNumberOperand(Token operator, Object operand){
    if (operand.type == NUMBER) return NULL;
    runtime_error_flag = 1;
    return createRuntimeError(operator, "Operand must be a number.");
}

RuntimeError* checkNumberOperands(Token operator, Object left, Object right){
    if (left.type == NUMBER && right.type == NUMBER) return NULL;
    runtime_error_flag = 1;
    return createRuntimeError(operator, "Operands must be numbers.");
}


int isTruthy(Object* object){
    if (object->type == NIL){
        return 0;
    }
    if (object->type == TRUE || object->type == FALSE){
        return ((BoolValue*)object->value)->boolean;
    }
    return 1;
}

Object* quotientOperation(Object* left, Object* right){
    double left_value = (((NumberValue*)left->value)->number);
    double right_value = (((NumberValue*)right->value)->number);
    double result = left_value / right_value;
    char* buffer = (char*)malloc(sizeof(32));
    snprintf(buffer, 32, "%.6g", result);
    return createObject(NUMBER, buffer);
};
Object* multiplyOperation(Object* left, Object* right){
    double left_value = (((NumberValue*)left->value)->number);
    double right_value = (((NumberValue*)right->value)->number);
    double result = left_value * right_value;
    char* buffer = (char*)malloc(sizeof(32));
    snprintf(buffer, 32, "%.6g", result);
    return createObject(NUMBER, buffer);
};

Object* plusOperation(Object* left, Object* right){
    if (left->type == NUMBER && right->type == NUMBER){
        double left_value = (((NumberValue*)left->value)->number);
        double right_value = (((NumberValue*)right->value)->number);
        double result = left_value + right_value;
        char* buffer = (char*)malloc(sizeof(32));
        snprintf(buffer, 32, "%.6g", result);
        return createObject(NUMBER, buffer);    
    }
    if (left->type == STRING && right->type == STRING){
        char* left_value = (((StringValue*)left->value)->string);
        char* right_value = (((StringValue*)right->value)->string);
        char* buffer = (char*)malloc(strlen(left_value)*sizeof(char) + strlen(right_value)*sizeof(char) -1); 
        strcat(left_value, right_value);
        return createObject(STRING, left_value);
    }
    return NULL;
}

Object* minusOperation(Object* left, Object* right){
    double left_value = (((NumberValue*)left->value)->number);
    double right_value = (((NumberValue*)right->value)->number);
    double result = left_value - right_value;
    char* buffer = (char*)malloc(sizeof(32));
    snprintf(buffer, 32, "%.6g", result);
    return createObject(NUMBER, buffer);
}

int isGreater(double left, double right){
    return left > right;
}

int isGreaterEqual(double left, double right){
    return left >= right;
}

int isLess(double left, double right){
    return left < right;
}

int isLessEqual(double left, double right){
    return left <= right;
}

Object* relationalOperation(Object* left, Object* right, int (*comparison)(double, double)){
    double left_value = (((NumberValue*)left->value)->number);
    double right_value = (((NumberValue*)right->value)->number);
    TokenType result = comparison(left_value, right_value) ? TRUE : FALSE;
    char* literal = result == TRUE ? "TRUE": "FALSE";
    return createObject(result, literal);
}

TokenType isEqual(Object* left, Object* right){
    if (left->type == NIL && right->type == NIL) return TRUE;
    if (left->type != right->type) return FALSE;

    if (left->type == STRING){
        char* left_value = ((StringValue*)left->value)->string;
        char* right_value = ((StringValue*)right->value)->string;
        return strcmp(left_value, right_value) == 0 ? TRUE : FALSE;
    }
    if (left->type == NUMBER){
        double left_value = (((NumberValue*)left->value)->number);
        double right_value = (((NumberValue*)right->value)->number);
        return left_value == right_value ? TRUE: FALSE;   
    }
    if (left->type == TRUE && right->type == TRUE) return TRUE;
    if (left->type == FALSE && right->type == FALSE) return TRUE;
}

void* InterpreterVisitBinaryExpr(Visitor* self, Expr* expr){
    Interpreter* interpreter = (Interpreter*)self;
    ExprBinary* expr_binary = (ExprBinary*)expr;
    Object* left = evaluate(interpreter, expr_binary->left);
    Object* right = evaluate(interpreter, expr_binary->right);

    RuntimeError* runtime_error;
    switch (expr_binary->operator->type)
    {
        case MINUS:
            runtime_error = checkNumberOperands(*expr_binary->operator, *left, *right);
            if (runtime_error) return runtime_error;
            return minusOperation(left, right);
        case PLUS:
            Object* object = plusOperation(left, right);
            if (object) return object;

            runtime_error_flag = 1;
            runtime_error = createRuntimeError(*expr_binary->operator,
                                             "Operands must be two numbers or two strings.");
            // throw_exception(70, "Operands must be two numbers or two strings.");

            return runtime_error;
        case SLASH:
            runtime_error = checkNumberOperands(*expr_binary->operator, *left, *right);
            if (runtime_error) return runtime_error;

            return quotientOperation(left, right);
        case STAR:
            runtime_error = checkNumberOperands(*expr_binary->operator, *left, *right);
            if (runtime_error) return runtime_error;

            return multiplyOperation(left, right);
        case GREATER:
            runtime_error = checkNumberOperands(*expr_binary->operator, *left, *right);
            if (runtime_error) return runtime_error;

            return relationalOperation(left, right, isGreater);
        case GREATER_EQUAL:
            runtime_error = checkNumberOperands(*expr_binary->operator, *left, *right);
            if (runtime_error) return runtime_error;

            return relationalOperation(left, right, isGreaterEqual);
        case LESS:
            runtime_error = checkNumberOperands(*expr_binary->operator, *left, *right);
            if (runtime_error) return runtime_error;

            return relationalOperation(left, right, isLess);
        case LESS_EQUAL:
            runtime_error = checkNumberOperands(*expr_binary->operator, *left, *right);
            if (runtime_error) return runtime_error;

            return relationalOperation(left, right, isLessEqual);
        case BANG_EQUAL:
            return isEqual(left, right) == TRUE ? createObject(FALSE, "FALSE") : createObject(TRUE, "TRUE");
        case EQUAL_EQUAL:
            return isEqual(left, right) == TRUE ? createObject(TRUE, "TRUE") : createObject(FALSE, "FALSE");
    }
}

void interpret(struct Interpreter* self, Array* statements){
    for (int i = 0; i < statements->count; i++){
        Element* element = getElement(statements, i);
        Stmt* stmt;
        if (element->type == PRINT_STMT){
            stmt = (Stmt*)element->data.print_stmt;
        } else if (element->type == EXPRESSION_STMT) {
            stmt = (Stmt*)element->data.expr_stmt;
        }
        execute(self, stmt);
        if (runtime_error_flag){
            exit(70);
        }
    }

}

char* stringify(Object object){
    if (object.value == NULL) return "nil";

    if (object.type == NUMBER){
        double number = (((NumberValue*)object.value)->number);
        char* buffer = (char*)malloc(32);
        snprintf(buffer, 32, "%.6g", number);
        return buffer;
    }
    if (object.type == STRING) {
        return ((StringValue*)object.value)->string;
    }
    if (object.type == TRUE || object.type == FALSE) {
        return ((BoolValue*)object.value)->boolean != 0 ? "true" : "false";
    }
    exit(70);
    // fprintf(stderr, "Unknown type error\n");
    // exit(EXIT_FAILURE);
}


int endsWith(char *c, size_t c_size, char *end, size_t end_size){
    if (end_size > c_size) {
        return 0;
    }
    size_t start = c_size - end_size;
    for (int i = 0; i < end_size; i++){
        if (c[start+i] != end[i]) return 0;
    }
    return 1;
}


void initTokenList() {
    g_head_pointer = (Token *)malloc(sizeof(Token));
    g_tail_pointer = (Token *)malloc(sizeof(Token));
    g_head_pointer->next = g_tail_pointer;
    g_tail_pointer->prev = g_head_pointer;
}

int isEmptyList() {
  if (g_head_pointer->next == g_tail_pointer) {
    return 1;
  }
  return 0;
}

int releaseTokenList() {
  if (isEmptyList()) {
    return 1;
  } else {
    Token *now = g_head_pointer->next;
    while (now != g_tail_pointer) {
      Token *prev = now;
      now = now->next;
      free(prev);
    }
    free(g_head_pointer);
    free(g_tail_pointer);
    g_token_list_size = 0;
  }
}
Token* createToken(TokenType type, char *lexeme, char *literal, int line) {
    Token *new_token = (Token *)malloc(sizeof(Token));
    
    new_token->type = type;
    new_token->lexeme = strdup(lexeme);
    new_token->literal = strdup(literal);
    new_token->line = line;
    return new_token;
}

int insertAtTail(TokenType type, char *lexeme, char *literal, int line) {
    Token *new_token = (Token *)malloc(sizeof(Token));
    
    new_token->type = type;
    new_token->lexeme = strdup(lexeme);
    new_token->literal = strdup(literal);
    new_token->line = line;

    Token *original_last_node = g_tail_pointer->prev;
    g_tail_pointer->prev = new_token;
    new_token->prev = original_last_node;
    new_token->next = g_tail_pointer;
    original_last_node->next = new_token;
    g_token_list_size++;
    return 1;
}

int getSize() { return g_token_list_size; }

int copyStr(char *dest, char *src) {
    while (*src) {
    *dest = *src;
    src++;  
    dest++;
    }
    *dest = '\0';

    return 1;
}

void printToken(const Token *token) {
    if (token == NULL) return;

    const char *type_str = TokenTypeStrs[token->type];
    const char *lexeme_str = (strcmp(token->lexeme, "\0") == 0) ? "" : token->lexeme;
    const char *literal_str = (strcmp(token->literal, "\0") == 0) ? "null" : token->literal;

    printf("%s %s %s\n", type_str, lexeme_str, literal_str);
}

void printTokenList() {
    if (isEmptyList()) {
        printf("EOF  null\n");
        return;
    }

    Token *now = g_head_pointer;
    while (now->next != g_tail_pointer) {
        now = now->next;
        printToken(now);
    }
}

int isDigit(const char c){
    if (c >= '0' && c <='9') return 1;
    return 0;
}

int isAlpha(const char c){
    if (c >= 'a' && c <='z') return 1;
    if (c >= 'A' && c <= 'Z') return 1;
    if (c == '_') return 1;
    return 0;
}
int isAlphaNumeric(const char c){
    return isAlpha(c) || isDigit(c);
}


int isIn(const char *str, const char c){
    while (*str){
        if (*str == c) return 1;
        str++;
    }
    return 0;
}

void trimTrailingZeros(char *str){
    int is_decimal = 0;
    int decimal_index = 0;
    char *temp = str;
    while (*temp){
        if (*temp == '.'){
            is_decimal = 1;
            break;
        }
        temp++;
        decimal_index++;
    }

    int trim_index = strlen(str);
    for (int i = strlen(str) - 1; i > decimal_index; i--){
        if (str[i] != '0') break;
        trim_index = i;
    }
    if (trim_index == decimal_index + 1){
        str[trim_index + 1] = '\0';
        return;
    }
    str[trim_index] = '\0';
}

TokenType getReservedToken(const char *c){
    // TODO: [Refactor] Use hash table for efficiency in time
    char reserved_words[N_RESERVED_WORD][MAX_LEN_RESERVED_WORD] = {"and", "class", "else", "false", "for", "fun", "if", "nil", "or", "print", "return", "super", "this", "true", "var", "while"};
    TokenType reserved_word_tokens[N_RESERVED_WORD] = {AND, CLASS, ELSE, FALSE, FOR, FUN, IF, NIL, OR, PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE};
    for (int i = 0; i < N_RESERVED_WORD; i++){
        if (strcmp(c, reserved_words[i]) == 0){
            return reserved_word_tokens[i];
        }
    }
    return INVALID_TOKEN;
}

Token* peek(Parser* self) {
    return self->current;
}

int isAtEnd(Parser* self){
    if (peek(self)->type == END_OF_FILE){
        return 1;
    } else{
        return 0;
    }
}

Token* previous(Parser* self){
    return self->current->prev;
}

Token* advance(Parser* self){
    if (!isAtEnd(self)) self->current = peek(self)->next;
    return previous(self);
}

int check(Parser* self, TokenType type){
    if (isAtEnd(self)) return 0;
    if (peek(self)->type == type) return 1;
    return 0;
}

int match(Parser *self, TokenType* types, size_t num_types){
    for (int i = 0; i < num_types; i++){
        if (check(self, types[i])){
            advance(self);
            return 1;
        }
    }
    return 0;
}

Expr* primary(Parser *self){
    if (match(self, (TokenType[]){FALSE}, 1)){
        ExprLiteral* expr = malloc(sizeof(ExprLiteral));
        expr->base.accept = ExprLiteralAccept;
        expr->type = FALSE;
        expr->value = "false";
        return (Expr *)expr;
    }
    if (match(self, (TokenType[]){TRUE}, 1)){
        ExprLiteral* expr = malloc(sizeof(ExprLiteral));
        expr->base.accept = ExprLiteralAccept;
        expr->type = TRUE;
        expr->value = "true";
        return (Expr *)expr;
    }
    if (match(self, (TokenType[]){NIL}, 1)){
        ExprLiteral* expr = malloc(sizeof(ExprLiteral));
        expr->base.accept = ExprLiteralAccept;
        expr->type = NIL;
        expr->value = "nil";
        return (Expr *)expr;
    }
    // TODO: NUMBER, STRING 합치기
    if (match(self, (TokenType[]){NUMBER}, 1)){
        ExprLiteral* expr = malloc(sizeof(ExprLiteral));
        expr->base.accept = ExprLiteralAccept;
        expr->type = NUMBER;
        expr->value = previous(self)->literal;
        return (Expr *)expr;
    }
    if (match(self, (TokenType[]){STRING}, 1)){
        ExprLiteral* expr = malloc(sizeof(ExprLiteral));
        expr->base.accept = ExprLiteralAccept;
        expr->type = STRING;
        expr->value = previous(self)->literal;
        return (Expr *)expr;
    }

    if (match(self, (TokenType[]){LEFT_PAREN}, 1)){
        Expr* expr = expression(self);
        consume(self, RIGHT_PAREN, "Expect ')' after expression.");
        ExprGrouping* expr_grouping = malloc(sizeof(ExprGrouping));
        expr_grouping->base.accept = ExprGroupingAccept;
        expr_grouping->expression = expr;
        return (Expr *)expr_grouping;
    }
    if (match(self, (TokenType[]){IDENTIFIER}, 1)){
        // exit(65);
    }

    had_error = 1;
    // self->parserError(self, peek(self), "Expect expression.");
}

Expr* unary(Parser *self){
    if (match(self, (TokenType[]){BANG, MINUS}, 2)){
        Token* operator = previous(self);
        Expr* right = unary(self);
        ExprUnary* expr_unary = malloc(sizeof(ExprUnary));
        expr_unary->base.accept = ExprUnaryAccept;
        expr_unary->operator = operator;
        expr_unary->right = right;
        return (Expr *)expr_unary;
    }
    return primary(self);
}

Expr* factor(Parser *self){
    Expr* expr = unary(self);
    while (match(self, (TokenType[]){SLASH, STAR}, 2)){
        Token* operator = previous(self);
        Expr* right = unary(self);
        ExprBinary* expr_binary = malloc(sizeof(ExprBinary));
        expr_binary->base.accept = ExprBinaryAccept;
        expr_binary->left = expr;
        expr_binary->operator = operator;
        expr_binary->right = right;
        expr = (Expr*)expr_binary;
    }
    return expr;
}

Expr* term(Parser* self){
    Expr* expr = factor(self);

    while (match(self, (TokenType[]){MINUS, PLUS}, 2)){
        Token* operator = previous(self);
        Expr* right = factor(self);
        ExprBinary* expr_binary = malloc(sizeof(ExprBinary));
        expr_binary->base.accept = ExprBinaryAccept;
        expr_binary->left = expr;
        expr_binary->operator = operator;
        expr_binary->right = right;
        expr = (Expr*) expr_binary;
    }
    return expr;
}

Expr* comparison(Parser* self){
    Expr* expr = term(self);
    while(match(self, (TokenType[]){GREATER, GREATER_EQUAL, LESS, LESS_EQUAL}, 4)){
        Token* operator = previous(self);
        Expr* right = term(self);
        ExprBinary* expr_binary = malloc(sizeof(ExprBinary));
        expr_binary->base.accept = ExprBinaryAccept;
        expr_binary->left = expr;
        expr_binary->operator = operator;
        expr_binary->right = right;
        expr = (Expr*) expr_binary;
    }
    return expr;
}

Expr* equality(Parser *self){
    Expr* expr = comparison(self);

    while (match(self, (TokenType[]){BANG_EQUAL, EQUAL_EQUAL}, 2)){
        Token* operator = previous(self);
        Expr* right = comparison(self);
        ExprBinary* expr_binary= malloc(sizeof(ExprBinary));
        expr_binary->base.accept = ExprBinaryAccept;
        expr_binary->left = expr;
        expr_binary->operator = operator;
        expr_binary->right = right;
        expr = (Expr*) expr_binary;
    }
    return expr;
}

Expr* expression(Parser* self){
    return equality(self);
}

Stmt* statement(Parser* self){
    if (match(self, (TokenType[]){PRINT}, 1)) return printStatement(self);

    return expressionStatement(self);
}

Stmt* printStatement(Parser* self){
    Expr* value = expression(self);
    char* error_message = "Expect ';' after value.";
    consume(self, SEMICOLON, "");
    return (Stmt*)createPrintStmt(value); 
}

Stmt* expressionStatement(Parser *self){
    Expr* expr = expression(self);
    char* error_message = "Expect ';' after expression.";

    consume(self, SEMICOLON, "");
    return (Stmt*)createExpressionStmt(expr);
}

ParseError* parserError(Parser* self,Token* token, char* message){
    error(token, message);
    return createParseError();
}

void *synchronize(Parser* self){
    advance(self);

    while (!isAtEnd(self)){
        if (previous(self)->type == SEMICOLON) return NULL;

        switch(peek(self)->type){
            case CLASS:
            case FUN:
            case VAR:
            case FOR:
            case IF:
            case WHILE:
            case PRINT:
            case RETURN:
                return NULL;
        }
        advance(self);
    }
}

Array* parse(Parser* self){
    Array* stmt_array = createArray(INITIAL_LIST_SIZE);
    if (!stmt_array){
        fprintf(stderr, "Error: Initial memory allocation of Stmt list failed\n");
        exit(70);
    }
    while (!isAtEnd(self)) {
        Stmt* stmt = statement(self);
        
        Element element;
        if (stmt->accept == PrintStmtAccept){
            element.type = PRINT_STMT;
            element.data.print_stmt = (Print*)stmt;
        } else if (stmt->accept == ExpressionStmtAccept){
            element.type = EXPRESSION_STMT;
            element.data.expr_stmt = (Expression*)stmt;
        } else {
            fprintf(stderr, "Error: Unknown statements");
            exit(65);
        }
        addElement(stmt_array, element);
    }

    return stmt_array;
}


Token* consume(Parser* self, TokenType type, char* message){
    if (check(self, type)) return advance(self);
    if (type == SEMICOLON){
        runtime_error_flag = 1;
    } else {
        had_error = 1;
    }
    self->parserError(self, peek(self), message);
    return NULL;
}


Parser* createParser() {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    ParseError* parse_error = createParseError();
    if (parser) {
        parser->current = g_head_pointer->next;
        parser->isAtEnd = isAtEnd;
        parser->peek = peek;
        parser->advance = advance;
        parser->check = check;
        parser->previous = previous;
        parser->match = match;
        parser->primary = primary;
        parser->unary = unary;
        parser->factor = factor;
        parser->term = term;
        parser->comparison = comparison;
        parser->equality = equality;
        parser->expression = expression;
        parser->statement = statement;
        parser->parserError = parserError;
        parser->parse = parse;
    }
    return parser;
}

ParseError* createParseError() {
    ParseError* parse_error = (ParseError*)malloc(sizeof(ParseError));
    if (parse_error){
        // TODO
    }
    return parse_error;
}


RuntimeError* createRuntimeError(Token token, char* message){
    RuntimeError* runtime_error = (RuntimeError*)malloc(sizeof(RuntimeError));
    if (runtime_error){
        runtime_error->token = token;
        runtime_error->message = message;
    }
    return runtime_error;
}

void* ExprBinaryAccept(Expr *self, Visitor *visitor){
    visitor->visitBinaryExpr(visitor, self);
}

void* ExprUnaryAccept(Expr *self, Visitor *visitor){
    visitor->visitUnaryExpr(visitor, self);
}

void* ExprGroupingAccept(Expr *self, Visitor *visitor){
    visitor->visitGroupingExpr(visitor, self);
}

void* ExprLiteralAccept(Expr *self, Visitor *visitor){
    visitor->visitLiteralExpr(visitor, self);
}

void report(int line, char* where, char* message){
    printf("[line %d] Error %s: %s\n", line, where, message);
    // had_error = 1;
}

void error(Token* token, char* message) {
    char where[MAX_TOKEN_LEXEME_SIZE + 10] = {0}; // 안전한 초기화

    if (token->type == END_OF_FILE) {
        snprintf(where, sizeof(where), "at end");
    } else {
        snprintf(where, sizeof(where), "at '%s'", token->lexeme);
    }

    report(token->line, where, message);
}

char *parenthesize(const char *name, char **exprs, int count){
    size_t length = strlen(name) + 3; // '(' + ')' + '\0'
    for (int i = 0; i < count; i++){
        if (exprs[i]) length += strlen(exprs[i]) +1; 
    }

    char *result = malloc(length);
    strcpy(result, "(");
    strcat(result, name);

    for (int i = 0; i < count; i++){
        strcat(result, " ");
        strcat(result, exprs[i]);
    }

    strcat(result, ")");
    return result;
}

void *AstPrinterVisitBinaryExpr(Visitor *self, Expr *expr) {
    ExprBinary *binaryExpr = (ExprBinary *)expr;
    char *left = binaryExpr->left->accept(binaryExpr->left, self);
    char *right = binaryExpr->right->accept(binaryExpr->right, self);
    char *result = parenthesize(binaryExpr->operator->lexeme, (char *[]){left, right}, 2);
    free(left);
    free(right);
    return result;
}

void *AstPrinterVisitUnaryExpr(Visitor *self, Expr *expr){
    ExprUnary *unaryExpr = (ExprUnary *)expr;
    char *right = unaryExpr->right->accept(unaryExpr->right, self);
    char *result = parenthesize(unaryExpr->operator->lexeme, (char *[]){right}, 1);
    free(right);
    return result;
}

void *AstPrinterVisitGroupingExpr(Visitor *self, Expr *expr){
    ExprGrouping *groupingExpr = (ExprGrouping *)expr;
    char *expression = groupingExpr->expression->accept(groupingExpr->expression, self);
    char *result = parenthesize("group", (char *[]){expression}, 1);
    free(expression);
    return result;
}

void *AstPrinterVisitLiteralExpr(Visitor *self, Expr *expr){
    ExprLiteral *literalExpr = (ExprLiteral *)expr;
    if (!literalExpr -> value) return strdup("nil");
    return strdup(literalExpr->value);
}

char *print(Visitor *self, Expr *expr){
    return (char*)expr->accept(expr, self);
}

AstPrinter *newAstPrinter(){
    AstPrinter *printer = malloc(sizeof(AstPrinter));
    printer->base.visitBinaryExpr = AstPrinterVisitBinaryExpr;
    printer->base.visitUnaryExpr = AstPrinterVisitUnaryExpr;
    printer->base.visitGroupingExpr = AstPrinterVisitGroupingExpr;
    printer->base.visitLiteralExpr = AstPrinterVisitLiteralExpr;
    printer->print = print;
    return printer;
}

Interpreter *createInterpreter(){
    Interpreter *interpreter = (Interpreter*)malloc(sizeof(Interpreter));
    interpreter->base.visitLiteralExpr = InterpreterVisitLiteralExpr;
    interpreter->base.visitGroupingExpr = InterpreterVisitGroupingExpr;
    interpreter->base.visitBinaryExpr = InterpreterVisitBinaryExpr;
    interpreter->base.visitUnaryExpr = InterpreterVisitUnaryExpr;
    interpreter->stmt_visitor.visitExpressionStmt = InterpreterVisitExpressionStmt;
    interpreter->stmt_visitor.visitPrintStmt = InterpreterVisitPrintStmt;
    interpreter->evaluate = evaluate;
    interpreter->execute = execute;
    interpreter->interpret = interpret;
    return interpreter;
}

Array* createArray(size_t initialCapacity) {
    Array* array = (Array*)malloc(sizeof(Array));
    array->count = 0;
    array->capacity = initialCapacity;
    array->elements = (Element*)malloc(sizeof(Element) * initialCapacity);
    return array;
}

void addElement(Array* array, Element element) {
    if (array->count >= array->capacity) {
        array->capacity *= 2;
        array->elements = realloc(array->elements, sizeof(Element) * array->capacity);
    }
    array->elements[array->count++] = element;
}

void* getElement(Array* array, size_t index) {
    if (index >= array->count) {
        fprintf(stderr, "Index out of bounds.\n");
        return NULL;
    }
    return &array->elements[index];
}

void releaseArray(Array* array) {
    free(array->elements);
    free(array);
}

void* PrintStmtAccept(Stmt *self, StmtVisitor *stmt_visitor){
    stmt_visitor->visitPrintStmt(stmt_visitor, self);
};


void* ExpressionStmtAccept(Stmt *self, StmtVisitor *stmt_visitor){
    stmt_visitor->visitExpressionStmt(stmt_visitor, self);
};


Print* createPrintStmt(Expr* expr){
    Print* print_stmt = (Print*)malloc(sizeof(Print));
    if (!print_stmt){
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    // TODO: visitPrintStmt
    print_stmt->base.accept = PrintStmtAccept;
    print_stmt->expression = expr;
    return print_stmt;
}

Expression* createExpressionStmt(Expr* expr){
    Expression* expression_stmt = (Expression*)malloc(sizeof(Expression));
    if (!expression_stmt){
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    expression_stmt->base.accept = ExpressionStmtAccept;
    expression_stmt->expression = expr;
    return expression_stmt;
}

void* InterpreterVisitExpressionStmt(StmtVisitor *self, Stmt* stmt){
    Expression* expr_stmt = (Expression*)stmt;
    size_t offset = offsetof(Interpreter, stmt_visitor);

    Object* value = evaluate((Interpreter*)((char*)self - offset), expr_stmt->expression);

    return NULL;
};


void* InterpreterVisitPrintStmt(StmtVisitor *self, Stmt* stmt){
    Print* print_stmt = (Print*)stmt;

    size_t offset = offsetof(Interpreter, stmt_visitor);
    Object* value = evaluate((Interpreter*)((char*)self - offset), print_stmt->expression); 
    printf("%s\n", stringify(*value));
    return NULL;
}


Object* createObject(TokenType type,  const char* literal){
    Object* object = (Object*)malloc(sizeof(Object));
    if (!object) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    if (type == NUMBER){
        object->type = NUMBER;
        NumberValue* num = (NumberValue*)malloc(sizeof(NumberValue));
        num->number = strtod(literal, NULL); // 문자열 -> 숫자 변환
        object->value = num;
        return object;
    }

    // Boolean 타입 판별
    if (type == TRUE){
        object->type = TRUE;
        BoolValue* boolVal = (BoolValue*)malloc(sizeof(BoolValue));
        boolVal->boolean = 1;
        object->value = boolVal;
        return object;
    }
    if (type == FALSE){
        object->type = FALSE;
        BoolValue* boolVal = (BoolValue*)malloc(sizeof(BoolValue));
        boolVal->boolean = 0;
        object->value = boolVal;
        return object;
    }

    // Nil 타입 판별
    if (type == NIL){
        object->type = NIL;
        object->value = NULL;
        return object;
    }

    if (type == STRING){
        object->type = STRING;
        StringValue* str = (StringValue*)malloc(sizeof(StringValue));
        str->string = strdup(literal); // 문자열 복사
        object->value = str;
        return object;
    }
}