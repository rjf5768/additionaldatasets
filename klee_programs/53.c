#include <klee/klee.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

struct Stack {
    char stack[10];
    int top;
};
struct Stack st;

void push(char opd) {
    if(st.top == 9) {
        return; // Stack overflow - return instead of exit
    }
    st.top++;
    st.stack[st.top] = opd;
}

char pop() {
    char item;
    if(st.top == -1) {
        return '\0'; // Stack underflow - return null char instead of exit
    }
    item = st.stack[st.top];
    st.top--;
    return item;
}

uint16_t isEmpty() {
    if(st.top == -1) {
        return 1;
    }
    return 0;
}

char Top() {
    if (st.top == -1) {
        return '\0';
    }
    return st.stack[st.top];
}

int16_t priority(char opr) {
    if(opr == '+' || opr == '-') {
        return 0;
    }
    else if(opr == '/' || opr == '*' || opr == '%') {
        return 1;
    }
    else {
        return -1;
    }
}

char *convert(char inf[]) {
    static char post[25];
    int i;
    int j = 0;
    
    for(i = 0; i < strlen(inf); i++) {
        if(isalnum(inf[i])) {
            post[j] = inf[i];
            j++;
        }
        else if(inf[i] == '(') {
            push(inf[i]);
        }
        else if(inf[i] == ')') {
            while(Top() != '(' && !isEmpty()) {
                post[j] = pop();
                j++;
            }
            if (!isEmpty()) {
                pop(); // pop opening parentheses
            }
        }
        else {
            while( (!isEmpty()) && (priority(inf[i]) <= priority(Top())) && Top() != '(') {
                post[j] = pop();
                j++;
            }
            push(inf[i]);
        }
    }
    while(!isEmpty()) {
        post[j] = pop();
        j++;
    }
    post[j] = '\0';
    return post;
}

int main() {
    char inf[15];
    
    // Make input symbolic
    klee_make_symbolic(inf, sizeof(inf), "infix");
    
    // Ensure string is null-terminated
    inf[14] = '\0';
    
    // Constrain input to valid characters
    for (int i = 0; i < 14; i++) {
        klee_assume(inf[i] >= 32 && inf[i] <= 126);
    }
    
    // Initialize stack
    st.top = -1;
    
    // Convert infix to postfix
    char *post = convert(inf);
    
    // Verify output is not NULL
    klee_assert(post != NULL);
    
    // Verify output is null-terminated
    klee_assert(post[strlen(post)] == '\0');
    
    // Verify output length is reasonable
    klee_assert(strlen(post) <= 25);
    
    // Verify that all alphanumeric characters from input appear in output
    int input_alnum = 0, output_alnum = 0;
    for (int i = 0; i < strlen(inf); i++) {
        if (isalnum(inf[i])) input_alnum++;
    }
    for (int i = 0; i < strlen(post); i++) {
        if (isalnum(post[i])) output_alnum++;
    }
    klee_assert(input_alnum == output_alnum);
    
    // Verify that operators have valid priorities
    for (int i = 0; i < strlen(post); i++) {
        if (post[i] == '+' || post[i] == '-' || post[i] == '*' || post[i] == '/' || post[i] == '%') {
            klee_assert(priority(post[i]) >= 0);
        }
    }
    
    return 0;
} 