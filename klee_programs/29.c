#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <klee/klee.h>

#define MAX_EXPRESSION_LENGTH 100
#define MAX_STACK_SIZE 50

typedef struct {
    char data[MAX_STACK_SIZE];
    int top;
} Stack;

// Initialize stack
void init_stack(Stack* stack) {
    stack->top = -1;
}

// Check if stack is empty
int is_empty(Stack* stack) {
    return stack->top == -1;
}

// Check if stack is full
int is_full(Stack* stack) {
    return stack->top == MAX_STACK_SIZE - 1;
}

// Push element onto stack
void push(Stack* stack, char value) {
    if (!is_full(stack)) {
        stack->data[++stack->top] = value;
    }
}

// Pop element from stack
char pop(Stack* stack) {
    if (!is_empty(stack)) {
        return stack->data[stack->top--];
    }
    return '\0';
}

// Peek at top element
char peek(Stack* stack) {
    if (!is_empty(stack)) {
        return stack->data[stack->top];
    }
    return '\0';
}

// Get operator precedence
int get_precedence(char op) {
    switch (op) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        case '^':
            return 3;
        default:
            return 0;
    }
}

// Check if character is an operator
int is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

// Check if character is an operand (letter or digit)
int is_operand(char c) {
    return isalpha(c) || isdigit(c);
}

// Check if character is a letter
int is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Check if character is a digit
int is_digit(char c) {
    return c >= '0' && c <= '9';
}

// Convert infix to postfix
int infix_to_postfix(const char* infix, char* postfix) {
    Stack operator_stack;
    init_stack(&operator_stack);
    
    int postfix_index = 0;
    int infix_length = strlen(infix);
    
    for (int i = 0; i < infix_length; i++) {
        char c = infix[i];
        
        // Skip whitespace
        if (c == ' ' || c == '\t') {
            continue;
        }
        
        // If character is an operand, add to output
        if (is_operand(c)) {
            postfix[postfix_index++] = c;
            postfix[postfix_index++] = ' ';
        }
        // If character is an opening parenthesis, push to stack
        else if (c == '(') {
            push(&operator_stack, c);
        }
        // If character is a closing parenthesis, pop until matching '('
        else if (c == ')') {
            while (!is_empty(&operator_stack) && peek(&operator_stack) != '(') {
                postfix[postfix_index++] = pop(&operator_stack);
                postfix[postfix_index++] = ' ';
            }
            
            if (!is_empty(&operator_stack) && peek(&operator_stack) == '(') {
                pop(&operator_stack); // Remove '('
            } else {
                return 0; // Mismatched parentheses
            }
        }
        // If character is an operator
        else if (is_operator(c)) {
            while (!is_empty(&operator_stack) && 
                   peek(&operator_stack) != '(' && 
                   get_precedence(peek(&operator_stack)) >= get_precedence(c)) {
                postfix[postfix_index++] = pop(&operator_stack);
                postfix[postfix_index++] = ' ';
            }
            push(&operator_stack, c);
        }
    }
    
    // Pop remaining operators
    while (!is_empty(&operator_stack)) {
        char op = pop(&operator_stack);
        if (op == '(') {
            return 0; // Mismatched parentheses
        }
        postfix[postfix_index++] = op;
        postfix[postfix_index++] = ' ';
    }
    
    // Remove trailing space
    if (postfix_index > 0) {
        postfix[postfix_index - 1] = '\0';
    } else {
        postfix[0] = '\0';
    }
    
    return 1; // Success
}

// Evaluate postfix expression
int evaluate_postfix(const char* postfix) {
    Stack operand_stack;
    init_stack(&operand_stack);
    
    int postfix_length = strlen(postfix);
    
    for (int i = 0; i < postfix_length; i++) {
        char c = postfix[i];
        
        if (c == ' ' || c == '\t') {
            continue;
        }
        
        if (is_digit(c)) {
            push(&operand_stack, c - '0');
        } else if (is_operator(c)) {
            if (operand_stack.top < 1) {
                return 0; // Not enough operands
            }
            
            int b = pop(&operand_stack);
            int a = pop(&operand_stack);
            int result;
            
            switch (c) {
                case '+':
                    result = a + b;
                    break;
                case '-':
                    result = a - b;
                    break;
                case '*':
                    result = a * b;
                    break;
                case '/':
                    if (b == 0) {
                        return 0; // Division by zero
                    }
                    result = a / b;
                    break;
                case '^':
                    result = 1;
                    for (int j = 0; j < b; j++) {
                        result *= a;
                    }
                    break;
                default:
                    return 0;
            }
            
            push(&operand_stack, result);
        }
    }
    
    if (operand_stack.top == 0) {
        return operand_stack.data[0];
    } else {
        return 0; // Invalid expression
    }
}

// Check if expression is valid
int is_valid_expression(const char* expression) {
    int parentheses_count = 0;
    int length = strlen(expression);
    
    for (int i = 0; i < length; i++) {
        char c = expression[i];
        
        if (c == '(') {
            parentheses_count++;
        } else if (c == ')') {
            parentheses_count--;
            if (parentheses_count < 0) {
                return 0; // Unmatched closing parenthesis
            }
        }
    }
    
    return parentheses_count == 0;
}

// Count operators in expression
int count_operators(const char* expression) {
    int count = 0;
    int length = strlen(expression);
    
    for (int i = 0; i < length; i++) {
        if (is_operator(expression[i])) {
            count++;
        }
    }
    
    return count;
}

// Count operands in expression
int count_operands(const char* expression) {
    int count = 0;
    int length = strlen(expression);
    
    for (int i = 0; i < length; i++) {
        if (is_operand(expression[i])) {
            count++;
        }
    }
    
    return count;
}

// Check if expression is balanced (operators and operands)
int is_balanced_expression(const char* expression) {
    int operands = count_operands(expression);
    int operators = count_operators(expression);
    
    // For a valid infix expression, operands = operators + 1
    return operands == operators + 1;
}

// KLEE test function
int main() {
    char infix[MAX_EXPRESSION_LENGTH];
    char postfix[MAX_EXPRESSION_LENGTH];
    
    // Make infix expression symbolic
    klee_make_symbolic(infix, sizeof(infix), "infix");
    
    // Constrain input to valid characters
    for (int i = 0; i < MAX_EXPRESSION_LENGTH - 1; i++) {
        klee_assume(infix[i] == '\0' || 
                   infix[i] == ' ' || 
                   infix[i] == '(' || 
                   infix[i] == ')' || 
                   infix[i] == '+' || 
                   infix[i] == '-' || 
                   infix[i] == '*' || 
                   infix[i] == '/' || 
                   infix[i] == '^' || 
                   (infix[i] >= '0' && infix[i] <= '9') ||
                   (infix[i] >= 'a' && infix[i] <= 'z') ||
                   (infix[i] >= 'A' && infix[i] <= 'Z'));
    }
    infix[MAX_EXPRESSION_LENGTH - 1] = '\0';
    
    // Test stack operations
    Stack test_stack;
    init_stack(&test_stack);
    
    klee_assert(is_empty(&test_stack));
    klee_assert(!is_full(&test_stack));
    
    push(&test_stack, 'a');
    klee_assert(!is_empty(&test_stack));
    klee_assert(peek(&test_stack) == 'a');
    
    char popped = pop(&test_stack);
    klee_assert(popped == 'a');
    klee_assert(is_empty(&test_stack));
    
    // Test operator precedence
    klee_assert(get_precedence('+') == 1);
    klee_assert(get_precedence('-') == 1);
    klee_assert(get_precedence('*') == 2);
    klee_assert(get_precedence('/') == 2);
    klee_assert(get_precedence('^') == 3);
    klee_assert(get_precedence('a') == 0);
    
    // Test character classification
    klee_assert(is_operator('+'));
    klee_assert(is_operator('-'));
    klee_assert(is_operator('*'));
    klee_assert(is_operator('/'));
    klee_assert(is_operator('^'));
    klee_assert(!is_operator('a'));
    
    klee_assert(is_operand('a'));
    klee_assert(is_operand('5'));
    klee_assert(!is_operand('+'));
    
    klee_assert(is_letter('a'));
    klee_assert(is_letter('Z'));
    klee_assert(!is_letter('0'));
    
    klee_assert(is_digit('0'));
    klee_assert(is_digit('9'));
    klee_assert(!is_digit('a'));
    
    // Test simple expressions
    const char* simple_infix = "a+b";
    int success = infix_to_postfix(simple_infix, postfix);
    klee_assert(success);
    klee_assert(strcmp(postfix, "a b +") == 0);
    
    const char* simple_infix2 = "a*b";
    success = infix_to_postfix(simple_infix2, postfix);
    klee_assert(success);
    klee_assert(strcmp(postfix, "a b *") == 0);
    
    // Test expression with parentheses
    const char* paren_infix = "(a+b)*c";
    success = infix_to_postfix(paren_infix, postfix);
    klee_assert(success);
    klee_assert(strcmp(postfix, "a b + c *") == 0);
    
    // Test evaluation
    int result = evaluate_postfix("1 2 +");
    klee_assert(result == 3);
    
    result = evaluate_postfix("2 3 *");
    klee_assert(result == 6);
    
    result = evaluate_postfix("1 2 + 3 *");
    klee_assert(result == 9);
    
    // Test validation
    klee_assert(is_valid_expression("a+b"));
    klee_assert(is_valid_expression("(a+b)*c"));
    klee_assert(!is_valid_expression("(a+b"));
    klee_assert(!is_valid_expression("a+b)"));
    
    // Test counting
    klee_assert(count_operators("a+b*c") == 2);
    klee_assert(count_operands("a+b*c") == 3);
    
    // Test balance
    klee_assert(is_balanced_expression("a+b"));
    klee_assert(is_balanced_expression("a+b*c"));
    klee_assert(!is_balanced_expression("a+b+"));
    klee_assert(!is_balanced_expression("+a+b"));
    
    // Test with symbolic input
    if (is_valid_expression(infix) && is_balanced_expression(infix)) {
        success = infix_to_postfix(infix, postfix);
        if (success) {
            // Check that postfix has correct number of operators and operands
            int infix_ops = count_operators(infix);
            int infix_operands = count_operands(infix);
            int postfix_ops = count_operators(postfix);
            int postfix_operands = count_operands(postfix);
            
            klee_assert(infix_ops == postfix_ops);
            klee_assert(infix_operands == postfix_operands);
        }
    }
    
    // Test edge cases
    const char* empty_infix = "";
    success = infix_to_postfix(empty_infix, postfix);
    klee_assert(success);
    klee_assert(strlen(postfix) == 0);
    
    const char* single_infix = "a";
    success = infix_to_postfix(single_infix, postfix);
    klee_assert(success);
    klee_assert(strcmp(postfix, "a") == 0);
    
    // Test complex expression
    const char* complex_infix = "a+b*c-d";
    success = infix_to_postfix(complex_infix, postfix);
    klee_assert(success);
    
    // Test with mixed letters and digits
    const char* mixed_infix = "x1+y2*z3";
    success = infix_to_postfix(mixed_infix, postfix);
    klee_assert(success);
    
    return 0;
} 