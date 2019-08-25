#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static void exec_lambda();

#define STACK_SIZE 100

struct Tokens {
    int element;
    int num_or_sym;
};
struct Tokens stack[STACK_SIZE];
int top = -1;

char int_string[100];
int iter = 0;
int empty_int_string = 1;

int in_a_lambda = 0;
int running_lambda = 0;

int lambdas[STACK_SIZE] = { 0 };
int current_lambda = 0;
char *line;
int curr_i;

// Keep track of functions
// so we don't execute one multiple times
int excl_num = 0;
int func_num = 0;
int ques_num = 0;

/*
Counts left & right brackets

TODO: Consolidate with l_bracket and
r_bracket in exec_lambda()
*/
int lb = 0;
int rb = 0;

int is_empty() { (top == -1) ? (1) : (0); }
int is_full() { (top == STACK_SIZE) ? (1) : (0); }

struct Tokens peek() {
    return stack[top];
}

struct Tokens pop() {
    struct Tokens single_token;
    if (!is_empty()) {
        single_token = stack[top];
        top = top - 1;
        if (single_token.num_or_sym == 0) {
            //printf("POPPED %c\n", single_token.element);
        } else {
            //printf("POPPED %d\n", single_token.element);
        }
        return single_token;
    } else {
        //printf("ERR: STACK EMPTY\n");
    }
}

void push(int data, int type) {
    if (!is_full()) {
        top = top + 1;
        stack[top].element = data;
        stack[top].num_or_sym = type;
        if (type == 0) {
            //printf("PUSHED %c\n", data);
        } else {
            //printf("PUSHED %d\n", data);
        }
    } else {
        //printf("ERR: STACK FULL\n");
    }
}

char *enter_input(void) {
    char * line = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer for us
    getline( & line, & bufsize, stdin);
    return line;
}

void view_stack_contents() {
    for (int i = 0; i <= top; i++) {
        printf("%d ", stack[i].element);
    }
    printf("\n");
}

void eval(char symbol) {
    switch (symbol) {
    case '+':
        {
            int int_first = pop().element;
            int int_second = pop().element;
            push((int_first + int_second), 1);
            break;
        }

    case '*':
        {
            int int_first = pop().element;
            int int_second = pop().element;
            push((int_first * int_second), 1);
            break;
        }

    case '-':
        {
            int int_first = pop().element;
            int int_second = pop().element;
            push((int_second - int_first), 1);
            break;
        }

    case '/':
        {
            int int_first = pop().element;
            int int_second = pop().element;
            push((int_second / int_first), 1);
            break;
        }

    // Duplicate top-most stack item
    case '$':
        {
            int int_first = pop().element;
            push(int_first, 1);
            push(int_first, 1);
            break;
        }

    // Delete top-most stack item
    case '%':
        {
            int int_first = pop().element;
            break;
        }

    // Make negative
    case '_':
        {
            int int_first = pop().element;
            int_first *= -1;
            push(int_first, 1);
            break;
        }

    // Swap to topmost stack-items
    case '\\':
        {
            int int_first = pop().element;
            int int_second = pop().element;
            push(int_first, 1);
            push(int_second, 1);
            break;
        }

    // Rotate 3rd stack item to top
    case '@':
        {
            int int_first = pop().element;
            int int_second = pop().element;
            int int_third = pop().element;
            push(int_second, 1);
            push(int_first, 1);
            push(int_third, 1);
            break;
        }

    // Equality
    case '=':
        {
            int int_first = pop().element;
            int int_second = pop().element;

            if (int_first == int_second) {
                push(-1, 1);
            } else {
                push(0, 1);
            }
            break;
        }

    // Greater-than
    case '>':
        {
            int int_first = pop().element;
            int int_second = pop().element;

            if (int_first > int_second) {
                push(0, 1);
            } else {
                push(-1, 1);
            }
            break;
        }

    // Conditional
    // NOTE: Executes if non-zero
    case '?':
        {
            if (ques_num <= func_num) {
                if ((!in_a_lambda) || (!running_lambda)) {
                    int int_first = pop().element;
                    if (int_first != 0) {
                    	//printf("??? %d\n", curr_i);
                        exec_lambda();
                    }
                }
            }
            ques_num++;
            break;
        }

    // Execute lambda
    case '!':
        {
            if (excl_num <= func_num) {
                if ((!in_a_lambda) || (!running_lambda)) {
                    //printf("!!! %d\n", curr_i);
                    exec_lambda();
                }
            }
            excl_num++;
            break;
        }

    case '[':
        {
            lb++;
            func_num++;
            
            in_a_lambda = 1;
            lambdas[current_lambda] = curr_i;
            current_lambda++;
            //printf("FOUND LAMBDA AT %d\n", lambdas[current_lambda - 1]);
            
            break;
        }

    case ']':
        {
            in_a_lambda = 0;
            break;
        }

    // Write top of stack as decimal integer
    // NOTE: It does not peek, but pops
    case '.':
        {
            int int_first = pop().element;
            printf("%d\n", int_first);
            break;
        }
    }
}

int parse(char chr, int loc) {
    curr_i = loc;
    if (isdigit(chr) && (!in_a_lambda)) {
        int_string[iter] = chr;
        iter++;
        empty_int_string = 0;
    } else if (!in_a_lambda) {
        // Push the combined integer
        if (!empty_int_string) {
            push(atoi(int_string), 1);
        }

        // Reset building of integer string
        memset(int_string, 0, 100);
        empty_int_string = 1;
        iter = 0;

        // Evaluate symbols
        if ((chr != '\n') && (chr != ' ')) {
            //push(line[i], 0);
            eval(chr);
        }
    } else if ((in_a_lambda) && (chr == '[')) {
        lb++;
    } else if ((in_a_lambda) && (chr == ']')) {
        rb++;
        if (lb == rb) {
            eval(chr);
        }
    }
}

void exec_lambda() {
	//printf("ENTERING LAMBDA FROM %d\n", curr_i);
	running_lambda = 1;
    in_a_lambda = 0;
    int loc = lambdas[current_lambda - 1] + 1; // Skip past leftmost [

    int l_bracket = 0;
    int r_bracket = 0;

    while(running_lambda) {
        if (line[loc] == '[') {
            l_bracket++;
        } else if (line[loc] == ']') {
            r_bracket++;
        }
        parse(line[loc], loc);

        if (r_bracket > l_bracket) {
            running_lambda = 0;
        }
        loc++;
    }
    lambdas[current_lambda] = 0;
    //printf("EXITING LAMBDA FROM %d\n", curr_i);
}

int read_input(char *a_line) {
    if ((a_line != NULL) && (a_line[0] == '\0')) {
        return 1;
    } else if (a_line[0] == '\n') {
        // If user enters an empty line, display stack elements
        view_stack_contents();
        return 1;
    } else {
        for (int i = 0; i < strlen(a_line); i++) {
            parse(a_line[i], i);
        }
        return 1;
    }
}

void interpret(void) {
    int status;
    do {
        printf("¬T ");
        line = enter_input();
        status = read_input(line);
        free(line);
    } while (status);
}

int main(int argc, char ** argv) {
    interpret();
    return 0;
}