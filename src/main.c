#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define MAX_TOKEN_SIZE 100

typedef enum TokenType {
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,

    STAR,
    DOT,
    COMMA,
    PLUS,
    MINUS,
    SEMICOLON,

    EQUAL,
    EQUAL_EQUAL,
    BANG,
    BANG_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,

    STRING,
    NUMBER,

    SLASH,

    END_OF_FILE,
} TokenType;

typedef struct Token {
    TokenType type;
    char *lexeme;
    char *literal;
    int line;
} Token;

char *read_file_contents(const char *filename);

void scanning(const char *file_contents);

int isDigit(const char c);

int isIn(const char *str, const char c);

void trimTrailingZeros(char *str);


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
        // You can use print statements as follows for debugging, they'll be visible when running tests.
        fprintf(stderr, "Logs from your program will appear here!\n");
        
        char *file_contents = read_file_contents(argv[2]);
        // Uncomment this block to pass the first stage
        if (strlen(file_contents) > 0) {
            scanning(file_contents);
            // fprintf(stderr, "Scanner not implemented\n");
            // exit(1);
        }
        if (strlen(file_contents) == 0){
            printf("EOF  null\n");
        } 
        
        free(file_contents);
    } else {
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

void scanning(const char *file_contents){
    const char *now = &file_contents[0];
    size_t file_size = strlen(file_contents);
    Token tokens[MAX_TOKEN_SIZE];
    int token_index = 0;
    int line = 1;
    int has_error = 0;
    for (int i = 0; i < file_size; i++){
        Token token;
        switch (*now) {
            case '(':
                token.type = LEFT_PAREN;
                token.lexeme = strdup("(");  // 문자열 복사
                token.literal = NULL;
                token.line = line;
                break;
            case ')':
                token.type = RIGHT_PAREN;
                token.lexeme = strdup(")");  // 문자열 복사
                token.literal = NULL;
                token.line = line;
                break;
            case '{':
                token.type = LEFT_BRACE;
                token.lexeme = strdup("{");
                token.literal = NULL;
                token.line = line;
                break;
            case '}':
                token.type = RIGHT_BRACE;
                token.lexeme = strdup("}");
                token.literal = NULL;
                token.line = line;
                break;
            case '*':
                token.type = STAR;
                token.lexeme = strdup("*");
                token.literal = NULL;
                token.line = line;
                break;
            case '.':
                token.type = DOT;
                token.lexeme = strdup(".");
                token.literal = NULL;
                token.line = line;
                break;
            case ',':
                token.type = COMMA;
                token.lexeme = strdup(",");
                token.literal = NULL;
                token.line = line;
                break;
            case '+':
                token.type = PLUS;
                token.lexeme = strdup("+");
                token.literal = NULL;
                token.line = line;
                break;
            case '-':
                token.type = MINUS;
                token.lexeme = strdup("-");
                token.literal = NULL;
                token.line = line;
                break;
            case '=':
                if (*(now + 1) == '='){
                    token.type = EQUAL_EQUAL;
                    token.lexeme = strdup("==");
                    token.literal = NULL;
                    token.line = line;
                    now++;
                    i++;
                } else{
                    token.type = EQUAL;
                    token.lexeme = strdup("=");
                    token.literal = NULL;
                    token.line = line;                    
                }
                break;
            case '!':
                if (*(now + 1) == '='){
                        token.type = BANG_EQUAL;
                        token.lexeme = strdup("!=");
                        token.literal = NULL;
                        token.line = line;
                        now++;
                        i++;
                } else{
                    token.type = BANG;
                    token.lexeme = strdup("!");
                    token.literal = NULL;
                    token.line = line;                    
                }
                break;            
            case '<':
                if (*(now + 1) == '='){
                        token.type = LESS_EQUAL;
                        token.lexeme = strdup("<=");
                        token.literal = NULL;
                        token.line = line;
                        now++;
                        i++;
                } else{
                    token.type = LESS;
                    token.lexeme = strdup("<");
                    token.literal = NULL;
                    token.line = line;                    
                }
                break; 
            case '>':
                if (*(now + 1) == '='){
                        token.type = GREATER_EQUAL;
                        token.lexeme = strdup(">=");
                        token.literal = NULL;
                        token.line = line;
                        now++;
                        i++;
                } else{
                    token.type = GREATER;
                    token.lexeme = strdup(">");
                    token.literal = NULL;
                    token.line = line;                    
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
                    token.type = SLASH;
                    token.lexeme = strdup("/");
                    token.literal = NULL;
                    token.line = line;
                    break; 
                }
            case '\t':
                now++;
                continue;
            case ' ':
                now++;
                continue;
            case ';':
                token.type = SEMICOLON;
                token.lexeme = strdup(";");
                token.literal = NULL;
                token.line = line;
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
                char lexeme[MAX_TOKEN_SIZE];
                int lexeme_length = end - start + 1;
                strncpy(lexeme, file_contents + start, lexeme_length);
                lexeme[lexeme_length] = '\0';

                char literal[MAX_TOKEN_SIZE];
                int literal_length = end - start - 1;
                strncpy(literal, file_contents + start + 1, literal_length);
                literal[literal_length] = '\0';

                token.type=STRING;
                token.lexeme = strdup(lexeme);
                token.literal = strdup(literal);
                token.line = line;
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

                    char lexeme[MAX_TOKEN_SIZE];
                    int lexeme_length = end - start + 1;
                    strncpy(lexeme, file_contents + start, lexeme_length);
                    lexeme[lexeme_length] = '\0';

                    token.type = NUMBER;
                    token.lexeme = strdup(lexeme);
                    if (isIn(lexeme, '.')){
                        trimTrailingZeros(lexeme);
                        token.literal = strdup(lexeme);
                    } else {
                        char literal[MAX_TOKEN_SIZE];
                        int literal_length = end - start + 1 + 2;
                        strncpy(literal, file_contents + start, literal_length);
                        literal[literal_length-2] = '.';
                        literal[literal_length-1] = '0';
                        literal[literal_length] = '\0';
                        token.literal = strdup(literal);
                    }
                    token.line = line;
                    break;
                }
                fprintf(stderr, "[line %d] Error: Unexpected character: %c\n", line, *now);
                now++;
                has_error = 1;
                continue; 
        }
        tokens[token_index++] = token; 
        now++;
    }


    for (int i = 0; i < token_index; i++) {
        const char *type_str;
        switch (tokens[i].type) {
            case LEFT_PAREN: type_str = "LEFT_PAREN"; break;
            case RIGHT_PAREN: type_str = "RIGHT_PAREN"; break;
            case LEFT_BRACE: type_str = "LEFT_BRACE"; break;
            case RIGHT_BRACE: type_str = "RIGHT_BRACE"; break;

            case STAR: type_str = "STAR"; break;
            case DOT: type_str = "DOT"; break;
            case COMMA: type_str = "COMMA"; break;
            case PLUS: type_str = "PLUS"; break;
            case MINUS: type_str = "MINUS"; break;
            case SEMICOLON: type_str = "SEMICOLON"; break;

            case EQUAL: type_str = "EQUAL"; break;
            case EQUAL_EQUAL: type_str = "EQUAL_EQUAL"; break;
            case BANG: type_str = "BANG"; break;
            case BANG_EQUAL: type_str = "BANG_EQUAL"; break;
            case LESS: type_str = "LESS"; break;
            case LESS_EQUAL: type_str = "LESS_EQUAL"; break;
            case GREATER: type_str = "GREATER"; break;
            case GREATER_EQUAL: type_str = "GREATER_EQUAL"; break;

            case STRING: type_str = "STRING"; break;
            case NUMBER: type_str = "NUMBER"; break;

            case SLASH: type_str = "SLASH"; break;

            case END_OF_FILE: type_str = "EOF"; break;
            default: type_str = "UNKNOWN"; break;
        }
        printf("%s %s %s\n", 
            type_str,
            tokens[i].lexeme,
            tokens[i].literal == NULL ? "null" : tokens[i].literal);
    }
    printf("EOF  null\n");

    // 할당된 메모리 해제
    for (int i = 0; i < token_index; i++) {
        free(tokens[i].lexeme);
    }

    if (has_error){
        exit(65);
    } else{
        exit(0);
    }
}

int isDigit(const char c){
    if (c >= '0' && c <='9') return 1;
    return 0;
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