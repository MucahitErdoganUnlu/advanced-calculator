#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"

#define MAXTOKENS 257 // max number of tokens can be 256. add null char to it
#define TOKENLENGTH 257 // a token have max length of 256. add null char to it
#define N 1000
#define LONG_LONG_BITS 64   // to calculate right and left rotations

char tokens[MAXTOKENS][TOKENLENGTH];
char main_str[N];  // postfix notation will be stored in this array
int  cur = 0;  // used for traversing tokens[][]

int numtokens;
int left_bracket = 0;   // if left bracket is met, increment by 1. if right bracket is met, decrement by 1.

int infunction = 0; // for binary functions (i.e. all functions except for not()), when left bracket is met
                    // increment by 1, when a comma is met, decrement by 1.
int anAssignment = 0;
int emptyInput = 0;
long long int result_of_calculate;

char trash_line[10000]; //after '%' char or an erroneous input in the form of a2e, take the rest to the trash_line array.

typedef struct {
    char value[TOKENLENGTH];
} Token;

/* about stack */
Token stack[MAXTOKENS]; // during the traversing of the postfix array, when met by a constant or an identifier, push them,
                        // when met by an operator pop one or two tokens from the stack
int top = -1;
void push(Token);
Token pop();

int calculate(char *, HashTable *);
int tokenize_input(char);
void reset();

int  expr(char *);
int  term(char *);
int  moreterms(char *);
int  factor(char *);
int  morefactors(char *);
int  is_integer(char *);

/* my additions */
int  assignment(char *);
int term2(char *);
int moreterms2(char *);
int term1(char *);
int moreterms1(char *);
int  assignfactor(char *);
/* end of my additions */

/* my additions */
int is_identifier(char *);
int is_function(char *);
int unary_function(char *);
/* end of my additions */

int main()
{

    HashTable *ht = create_table(CAPACITY);
    printf("> ");
    char cur_char;
    while ( (cur_char = getchar()) != EOF) {
        if(!tokenize_input(cur_char)){     // solves the issue of ab3c
            puts("Error!");
            fgets(trash_line, sizeof(trash_line), stdin); // get the rest of the erroneous line to help getchar()
            reset();
            printf("> ");
            continue;
        }

        // parse the expression
        if(assignment(main_str) && !emptyInput){
            int is_true_calculate = calculate(main_str, ht);
            if(is_true_calculate && !anAssignment){
                    printf("%lld\n", result_of_calculate);
            }else if(!anAssignment){
                puts("Error!");
            }else if(!is_true_calculate){
                puts("Error!");
            }
        }
        else if(emptyInput){
        }
        else {
            puts("Error!");
        }

        printf("> ");
        reset();
    }
    printf("\n");
   

   return(0) ; 
}

/*
 * take postfix notation as char array input. Take hashtable as input,
 * so that it can retrieve the values of identifiers, or assign them.
 * when an asssignfactor or a constant is encountered, directly push them to stack. If an identifier is encountered,
 * retrieve the value from the hash table and then push the value.
 * If an operator or a binary function is encountered, pop 2 tokens from the stack and do the corresponding operation.
 * If a unary operation is encountered, pop 1.
 */
int calculate(char *str, HashTable *ht) {
    char* token_name = strtok(str, " ");
    long long int result = 0;

    // If this is an assignment we push the first token and calculate the rest to assign its value to first token
    if(anAssignment){
        Token token;
        strcpy(token.value, token_name);
        push(token);
        token_name = strtok(NULL, " ");
    }
    while(token_name != NULL) {
        Token result_token;

        // Integer handler
        if(is_integer(token_name)){
            strcpy(result_token.value, token_name);
            push(result_token);
            token_name = strtok(NULL, " ");
            continue;
        }
        // Identifier handler
        if(is_identifier(token_name)){
            long long int value_of_identifier = print_search(ht, token_name);
            sprintf(result_token.value, "%lld", value_of_identifier);
            push(result_token);
            token_name = strtok(NULL, " ");
            continue;
        }

        // If assignment, set second pop as key and first pop as value
        if(strcmp(token_name, "=") == 0){
            if(top < 1){
                return 0;
            }
            char *y =pop().value;
            char *x =pop().value;
            ht_insert(ht, x, y);
            token_name = strtok(NULL, " ");
            return 1;
        }
        if(top < 0){
            return 0;
        }
        // Not function handler
        long long int y = atoll(pop().value);
        if(strcmp(token_name, "not") == 0){
            result = -1 - y;
            sprintf(result_token.value, "%lld", result);
            push(result_token);
            token_name = strtok(NULL, " ");
            continue;
        }
        long long int x = atoll(pop().value);

        // Handling the operations
        if(strcmp(token_name, "+") == 0){
            result = x + y;
        }
        else if(strcmp(token_name, "-") == 0){
            result = x - y;
        }
        else if(strcmp(token_name, "*") == 0){
            result = x * y;
        }
        else if(strcmp(token_name, "|") == 0){
            result = x | y;
        }
        else if(strcmp(token_name, "&") == 0){
            result = x & y;
        }
        // Handling the functions other than not
        else if(strcmp(token_name, "xor") == 0){
            result = x ^ y;
        }
        else if(strcmp(token_name, "ls") == 0){
            if(y < 0){
                return 0;
            }
            result = x << y;
        }
        else if(strcmp(token_name, "rs") == 0){
            if(y < 0){
                return 0;
            }
            result = x >> y;
        }
        else if(strcmp(token_name, "lr") == 0){
            if(y < 0){
                return 0;
            }
            result = (x << y) | (x >> (LONG_LONG_BITS - y));
        }
        else if(strcmp(token_name, "rr") == 0){
            if(y < 0){
                return 0;
            }
            result = (x >> y) | (x << (LONG_LONG_BITS - y));
        }
        // Push the result token in order to continue calculating the rest
        sprintf(result_token.value, "%lld", result);
        push(result_token);
        token_name = strtok(NULL, " ");
    }
    // Stack[0] is our result now
    result_of_calculate = atoll(stack[0].value);
    return 1;
}

// if the input contains error, return 0, otherwise return 1.
// In main function, we check the return value, and continue accordingly
// everytime a token is generated, return 1 and get called in main, in a while loop.
int tokenize_input(char cur_char){
    numtokens = 0 ;

    // when not met by line feed or '%' get the input char by char.
    while(cur_char != 10 && cur_char != '%'){   // ten equals line feed

        if(isspace(cur_char)){
            while(cur_char == ' '){
                cur_char = getchar();    // this handles the trailing blank spaces
            }
        }
        else if(isalpha(cur_char)){ // generate an identifier token
            int ix = 0;
            while(isalpha(cur_char)){
                tokens[numtokens][ix] = cur_char;
                ix++;
                cur_char = getchar();
            }
            if(isdigit(cur_char)){  // handles "a3e" type of wrong inputs
                return 0;
            }
            tokens[numtokens][ix] = '\0';   // end of token
            numtokens++;    // new token

        }
        else if(isdigit(cur_char)){ // generate a constant token
            int ix = 0;
            while(isdigit(cur_char)){
                tokens[numtokens][ix] = cur_char;
                ix++;
                cur_char = getchar();
            }
            if(isalpha(cur_char)){  // handles "1a1" type of wrong inputs
                return 0;
            }
            tokens[numtokens][ix] = '\0';   // end of token
            numtokens++;    // new token

        }
        else if(strchr("+-*&|(),=", cur_char) != NULL){  // generate an operator token
            tokens[numtokens][0] = cur_char;
            tokens[numtokens][1] = '\0';   // end of token
            cur_char = getchar();
            numtokens++;    // new token
        }else if(cur_char == 0){  // encountered ctrl+D
            return 1;
        }else{  // completely meaningless tokens
            return 0;
        }

    }
    if(cur_char == '%'){
        fgets(trash_line, sizeof(trash_line), stdin); // get the rest of comments to help getchar()
    }

    return 1;
}

// resets the state of the program, so that the code will continue to take new line inputs
void reset(){
    left_bracket = 0;
    numtokens = 0;
    cur = 0;
    infunction = 0;
    anAssignment = 0;
    top = -1;
    emptyInput = 0;
    result_of_calculate = 0;
    for(int i=0; i<MAXTOKENS; i++){
        for(int j=0; j<TOKENLENGTH; j++){
            tokens[i][j] = '\0';
        }
    }
    for(int i=0; i<256; i++){
        main_str[i] = '\0';
    }
}

/*
 * (1) this is the top function of the parsing algorithm.
 * (2) this function takes char *str as a parameter and the parsing result will be stored in the same array.
 * (3) Since it is passed by reference, the resultant argument will be available outside of this function.
 * (4) the function returns 1 if parsing is successful, otherwise returns 0. "Error!" will be displayed or code will
 * continue to calculate the result.
 * (5) return 0 if lower level parsing functions return 0.
 * All parsing functions share (2), (3), (4), (5) features. This is why these features will not be repeated
 * for every parsing function.
 */
int assignment(char *str){
    if(tokens[0][0] == '\0'){  // first token is empty. therefore, the rest of the tokens are empty as well.
        emptyInput =1;  // to display nothing and to continue taking input
        return 1;
    }

    int ix = 0;
    while(tokens[ix][0] != '\0'){
        if(tokens[ix][0] == '='){  // if the first char of tokens[ix] is not equal sign at any point, we will use expr()
            anAssignment = 1 ;
            break;
        }

        ix++;
    }
    // If the input does not contain '=' we can continue directly to the expr function
    if(!anAssignment){
        return expr(str);
    }
    else{  // contains '='
        char str1[N], str2[N], str3[N] ;

        str1[0] = str2[0] = str3[0] = '\0' ;

        // refer to (5). an important point is that no need to update cur variable because it is a global variable
        // and it is updated in lower level parsing functions if necessary
        if (!  assignfactor(str2)) {
            return(0) ;
        }

        if(tokens[cur][0] != '='){
            return 0;   // expecting '='
        }

        strcpy(str1,tokens[cur]) ;  // copy the equal sign
        cur++;

        // refer to (5)
        if (!  expr(str3)) {
            return(0) ;
        }

        // here, strings are manipulated according to postfix notation.
        // equal sign is placed at the end of the str so that it has the lowest precedence in postfix notation.
        strcat(str3,str1) ;
        strcat(str2,str3) ;
        strcpy(str,str2) ;
        return(1) ;
    }
}

/* refer to (2), (3), (4), (5).
 *
 */
int expr(char *str)
{
   char str1[N], str2[N] ; 
   
   str1[0] = str2[0] = '\0' ; 
   if (!  term2(str1)) {
      return(0) ; 
   } 
   if (!  moreterms2(str2)) {
      return(0) ; 
   }
   strcat(str1,str2) ; 
   strcpy(str,str1) ; 
   return(1) ;  
}

/* refer to (2), (3), (4), (5).
 * (6) this is an intermediary function so that our parsing algorithm is right-recursive.
 * Since (6) is shared with other intermediary parsing functions, it will not be repeated.
 */
int term2(char *str)
{
    char str1[N], str2[N] ;

    str1[0] = str2[0] = '\0' ;
    if (!  term1(str1)) {
        return(0) ;
    }
    if (!  moreterms1(str2)) {
        return(0) ;
    }
    // this string manipulation allows for right-recursion.
    strcat(str1,str2) ;
    strcpy(str,str1) ;
    return(1) ;
}

/* refer to (2), (3), (4), (5).
 * '|' operation handler
 */
int moreterms2(char *str)
{
    char str1[N], str2[N], str3[N] ;

    str1[0] = str2[0] = str3[0] = '\0' ;

    if ( (strcmp(tokens[cur],"|") == 0 ) ) {
        strcpy(str1,tokens[cur]) ;
        strcat(str1," ") ;
        cur++ ;
        if (!  term2(str2)) {
            return(0) ;
        }
        if (!  moreterms2(str3)) {
            return(0) ;
        }
    }
    // since '|' is the lowest operator after equal sign,
    // return 0 when any operator is encountered other than '='.
    // however, it can encounter ')' or ',', with the condition that left_bracket and infunction are not zero,
    // respectively
    else if((tokens[cur][0] != 0 && strchr(")," , tokens[cur][0]) == NULL)
    || (tokens[cur][0] == ')' && !left_bracket) || (tokens[cur][0] == ',' && !infunction)){

        return 0;
    }
    // since '|' has the lowest precedence in this level, it is placed at the end of the str.
    strcat(str2,str1) ;
    strcat(str2,str3) ;
    strcpy(str,str2) ;
    return(1) ;
}

/* refer to (2), (3), (4), (5), (6)
 *
 */
int term1(char *str)
{
    char str1[N], str2[N] ;

    str1[0] = str2[0] = '\0' ;
    if (!  term(str1)) {
        return(0) ;
    }
    if (!  moreterms(str2)) {
        return(0) ;
    }

    strcat(str1,str2) ;
    strcpy(str,str1) ;
    return(1) ;
}

/* refer to (2), (3), (4), (5), (6)
 * '&' operation handler
 */
int moreterms1(char *str)
{
    char str1[N], str2[N], str3[N] ;

    str1[0] = str2[0] = str3[0] = '\0' ;

    if ( (strcmp(tokens[cur],"&") == 0 ) ) {
        strcpy(str1,tokens[cur]) ;
        strcat(str1," ") ;
        cur++ ;
        if (!  term1(str2)) {
            return(0) ;
        }
        if (!  moreterms1(str3)) {
            return(0) ;
        }
    }
    // since '&' is the lowest operator in this level,
    // return 0 when any operator is encountered other than the operators that have lower precedence.
    // however, it can encounter ')' or ',', with the condition that left_bracket and infunction are not zero,
    // respectively
    else if((tokens[cur][0] != 0 && strchr("),|" , tokens[cur][0]) == NULL)
    || (tokens[cur][0] == ')' && !left_bracket) || (tokens[cur][0] == ',' && !infunction)){
        return 0;
    }
    // since '&' is the lowest operator in this level, place it at the end of the postfix.
    strcat(str2,str1) ;
    strcat(str2,str3) ;
    strcpy(str,str2) ;
    return(1) ;
}

/* refer to (2), (3), (4), (5), (6)
 *
 */
int term(char *str)
{
   char str1[N], str2[N] ; 
   
   str1[0] = str2[0] = '\0' ; 
   if (!  factor(str1)) {
      return(0) ; 
   } 
   if (!  morefactors(str2)) {
      return(0) ; 
   }

   strcat(str1,str2) ; 
   strcpy(str,str1) ; 
   return(1) ;  
}

/* refer to (2), (3), (4), (5), (6)
 * '+' and '-' operations handler
 */
int moreterms(char *str)
{
   char str1[N], str2[N], str3[N] ; 
   
   str1[0] = str2[0] = str3[0] = '\0' ; 
   
   if ( (strcmp(tokens[cur],"+") == 0 ) || (strcmp(tokens[cur],"-") == 0 ) ) {
       strcpy(str1,tokens[cur]) ; 
       strcat(str1," ") ; 
       cur++ ;  
       if (!  term(str2)) {
          return(0) ; 
       } 
       if (!  moreterms(str3)) {
         return(0) ; 
       }
   }
       // since '+' and '-' are the lowest operators in this level,
       // return 0 when any operator is encountered other than the operators that have lower precedence.
       // however, it can encounter ')' or ',', with the condition that left_bracket and infunction are not zero,
       // respectively
      else if((tokens[cur][0] != 0 && strchr("),&|" , tokens[cur][0]) == NULL) ||
      (tokens[cur][0] == ')' && !left_bracket) || (tokens[cur][0] == ',' && !infunction)){
       return 0;
   }
    // since '+' and '-' are the lowest operators in this level, place the current one at the end of the postfix.
   strcat(str2,str1) ;
   strcat(str2,str3) ;
   strcpy(str,str2) ; 
   return(1) ;  
}

/*
 * refer to (2), (3), (4), (5), (6)
 * this is the bottom parsing function of expr. here, we concatenate constants, identifiers, function keywords.
 * Besides, here parentheses are handled.
 */
int factor(char *str)
{
    if ( is_integer(tokens[cur])  ) {   // the token is a constant
       strcpy(str,tokens[cur]) ; 
       strcat(str," ") ; 
       cur++ ; 
       return(1) ; 
    }

    char str1[N], str2[N]  ;
    str1[0] = '\0', str2[0] = '\0' ;


    if ( is_function(tokens[cur])  ) {  // the token is a binary function
        infunction++;

        char keyword[5];  // a function has max 3 chars. add it to a blank space that will be concatenated and '\0'
        strcpy(keyword, tokens[cur]);
        strcat(keyword, " ");
//        keyword[4] = '\0';
        cur++ ;

        if(strcmp(tokens[cur],"(") != 0 ) {   // next token is not '('
            return 0;
        }
        left_bracket ++;
        cur++ ;

        if ( ! expr(str1) ) {
            return(0) ;
        }

        if ( strcmp(tokens[cur],",") != 0 ) {
            return(0) ;
        }
        infunction  --;
        cur++ ; // it is to increment after comma

        if ( ! expr(str2) ) {
            return(0) ;
        }

        if ( strcmp(tokens[cur],")") != 0 ) {
            return(0) ;
        }
        left_bracket --;
        cur++ ;     // increment after ')'
        strcpy(str,str1) ;      // have to concatenate str1, str2, and function keyword
        strcat(str,str2) ;
        strcat(str, keyword);
        return(1) ;
    }
    if ( unary_function(tokens[cur])  ) {   // the token is a unary function


        char keyword[5];        // unary function has 3 chars. add it to blank space and '\0'
        strcpy(keyword, tokens[cur]);
        strcat(keyword, " ");
        cur++ ;

        if(strcmp(tokens[cur],"(") != 0 ) {   // next token is not '('
            return 0;
        }
        left_bracket ++;
        cur++ ;
        if ( ! expr(str1) ) {
            return(0) ;
        }

        if ( strcmp(tokens[cur],")") != 0 ) {
            return(0) ;
        }
        left_bracket --;

        cur++ ;     // increment after ')'
        strcpy(str,str1) ;
        strcat(str, keyword);
        return(1) ;
    }

    if ( is_identifier(tokens[cur])  ) {    // the token is an identifier
        strcpy(str,tokens[cur]) ;
        strcat(str," ") ;
        cur++ ;
        return(1) ;
    }
    if ( strcmp(tokens[cur],"(") == 0 ) {
        left_bracket ++; // in upper levels, to handle wrong ')'
       cur++ ;
       if ( ! expr(str1) ) {
          return(0) ;    
       }
       if ( strcmp(tokens[cur],")") != 0 ) { 
          return(0) ;
       }
        left_bracket --; // in upper levels, to handle wrong ')'

        cur++ ;
       strcpy(str,str1) ; 
       return(1) ; 
    }
    return(0) ;
}

// '*' operation handler
int morefactors(char *str)
{
   char str1[N], str2[N], str3[N] ; 
   
   str1[0] = str2[0] = str3[0] = '\0' ; 
   
   if ( (strcmp(tokens[cur],"*") == 0 ) ) {
       strcpy(str1,tokens[cur]) ; 
       strcat(str1," ") ; 
       cur++ ;  
       if (!  factor(str2)) {
          return(0) ; 
       } 
       if (!  morefactors(str3)) {
         return(0) ; 
       }
   }
    // since morefactors function is the bottom of the function calls, it will be returned first.
    // when we put the '*' operator at the end of the postfix in the current level,
    // it will not be at the end of the postfix if upper level functions contains their own operators.
   strcat(str2,str3) ; 
   strcat(str2,str1) ; 
   strcpy(str,str2) ; 
   return(1) ;  
}

// since assignment can only happen with an identifier on LHS, we are only interested in identifiers.
int assignfactor(char *str)
{
    if(is_identifier(tokens[cur]) ){
        strcpy(str,tokens[cur]) ;
        strcat(str," ") ;
        cur++ ;
        return(1) ;
    }
    return(0) ;
}

/*
 * takes char array as input.
 * returns 1, if the token (i.e. a char array) is a constant
 */
int is_integer(char *token) 
{
    int isnumber = 1 ;
    char *q ; 

    for(q = token ; *q != '\0' ; q++) {
        isnumber = isnumber && isdigit(*q) ;
    }

    return(isnumber) ; 
}

/*
 * takes char array as input.
 * returns 1, if the token (i.e. a char array) satisfies the identifier rules
 */
int is_identifier(char *token){

   if(*token == '\0'){
        return 0;
    }
    if(is_function(token) || unary_function(token)){
        return 0;
    }
    int isidentifier = 1;
    char *q;

    for(q = token ; *q != '\0' ; q++) {
        isidentifier = isidentifier && isalpha(*q) ;
    }

    return isidentifier;
}

/*
 * takes char array as input.
 * returns 1, if the token (i.e. a char array) satisfies the binary function keyword rules
 */
int is_function(char *token){

/*    if(*token == '\0'){
        printf("this is an empty string being tested for function\n");
        return 0;
    } this should be unnecessary*/

    if(!strcmp(token,"xor") || !strcmp(token,"ls") || !strcmp(token,"rs")
       || !strcmp(token,"lr") || !strcmp(token,"rr") ){       // add other functions here
        return 1;
    }
    return 0;
}

/*
 * takes char array as input.
 * returns 1, if the token (i.e. a char array) satisfies the unary function keyword rules
 */
int unary_function(char *token){

    if(*token == '\0'){
        return 0;
    }

    if(!strcmp(token,"not")){       // add other functions here
        return 1;
    }
    return 0;
}

/*
 * takes Token struct as input.
 * puts the argument in the stack array.
 * because of the constraints in the description the case of overflow should not happen.
 */
void push(Token token)
{
    if (top == MAXTOKENS - 1)
    {
        printf("\nOverflow!!");
        exit(0);
    }
    else
    {
        top = top + 1;
        stack[top] = token;
    }
}

/*
 * returns the last token in the stack array.
 * decrements top by 1.
 * because of the checks that are made in calculation function, the case of underflow should not happen.
 * Instead, top value will be checked before the pop function call.
 */
Token pop()
{
    Token token;
    if (top == -1)
    {
        printf("\nUnderflow in stack!!");
        exit(0);
    }
    else
    {
        token = stack[top];
        top = top - 1;
    }
    return token;
}
