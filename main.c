#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

typedef enum {
	TOKEN_TYPE_NUM,
	TOKEN_TYPE_OPER
} token_type_e;

typedef enum {
	TOKEN_OPER_ADD,
	TOKEN_OPER_SUB,
	TOKEN_OPER_MUL,
	TOKEN_OPER_DIV,
	TOKEN_OPER_NOT,
	TOKEN_OPER_AND,
	TOKEN_OPER_OR,
	TOKEN_OPER_XOR,
	TOKEN_OPER_BOOL_NOT,
	TOKEN_OPER_SHIFT_LEFT,
	TOKEN_OPER_SHIFT_RIGHT,
	TOKEN_OPER_EQUAL,
	TOKEN_OPER_NOT_EQUAL,
	TOKEN_OPER_LESS_EQUAL,
	TOKEN_OPER_MORE_EQUAL,
	TOKEN_OPER_LESS,
	TOKEN_OPER_MORE,
	TOKEN_OPER_OPEN_PAREN,
	TOKEN_OPER_CLOSE_PAREN,
	TOKEN_OPER_STOP
} token_oper_e;

typedef struct {
	token_type_e type;
	union {
		float num;
		token_oper_e oper;
	} data;
} token_t;

void rewind_stdin(int amount, char *push_back) {

	// fseek doesn't work for some reason
	for(int i = 0; i < amount; i++) {
		ungetc(push_back[amount - i - 1], stdin);
	}
}

// get_char() is a hack to get stdin to work how i 
// want it to
int is_last_newline = 0;
char get_char() {
	char input[2] = { 0 };
	while(fgets(input, sizeof(input), stdin) == NULL) {
		if(is_last_newline) {
			return EOF;
		}
	}

	if(input[0] == '\n' || input[0] == '\r') {
		is_last_newline = 1;
	}
	
	return input[0];
}

void print_token(token_t token) {
	char *type_to_str[] = {
		"number",
		"operator"
	};

	if(token.type < 0 || token.type > (int)(TOKEN_TYPE_OPER)) {
		printf("token type is invalid\n");
		return;
	}
	
	printf("token type is %s\n", type_to_str[token.type]);
	switch(token.type) {
		case TOKEN_TYPE_NUM: {
			printf("token number is %f\n\n", token.data.num);
			return;
		}

		case TOKEN_TYPE_OPER: {
			char *oper_to_str[] = {
				"add",
				"subtract",
				"multiply",
				"divide",
				"bit not",
				"bit and",
				"bit or",
				"bit xor",
				"bool not",
				"shift left",
				"shift right",
				"is equal",
				"is not equal",
				"is less or equal",
				"is more or equal",
				"is less",
				"is more",
				"open parenthesis",
				"close parenthesis",
				"stop"
			};

			int oper = (int)(token.data.oper);
			if(oper < 0 || oper > TOKEN_OPER_STOP) {
				printf("token operator is invalid\n");
				return;
			}

			printf("token operator is %s\n\n", oper_to_str[oper]);
		}
	}
}

int power_ten_fit_int(int to_fit) {
	if(to_fit == 0) {
		return 0;
	}
	
	int exponent = 1;
	while(to_fit > exponent) {
		exponent *= 10;
	}

	return exponent;
}

void fetch_int(int *num) {
	int result = 0;
	char int_char = get_char();
	while(int_char >= '0' && int_char <= '9') {
		result *= 10;
		result += int_char - '0';
		int_char = get_char();
	}

	rewind_stdin(1, &int_char);
	*num = result;
}

void fetch_float(float *num) {
	int int_part = 0;
	int deci_part = 0;
	fetch_int(&int_part);
	char test_char = get_char();
	if(test_char == '.') {
		fetch_int(&deci_part);
	} else {
		rewind_stdin(1, &test_char);
	}

	*num = (
		(float)(int_part) + 
		(float)(power_ten_fit_int(deci_part))
	);
}

#define OPER_CHAR_MAX 2
int str_first_space(char *str) {
	int i = 0;
	for(
		i = 0; 
		str[i] != ' ' && 
		str[i] != 0 &&
		i < OPER_CHAR_MAX; 
		i++
	);
	
	return i;
}

#define ARRAY_SIZE(_array) \
	((int)(sizeof(_array) / sizeof((_array)[0])))

void fetch_oper(token_oper_e *oper) {
	token_oper_e char_to_oper[] = {
		TOKEN_OPER_ADD,
		TOKEN_OPER_SUB,
		TOKEN_OPER_MUL,
		TOKEN_OPER_DIV,
		TOKEN_OPER_NOT,
		TOKEN_OPER_AND,
		TOKEN_OPER_OR,
		TOKEN_OPER_XOR,
		TOKEN_OPER_BOOL_NOT,
		TOKEN_OPER_SHIFT_LEFT,
		TOKEN_OPER_SHIFT_RIGHT,
		TOKEN_OPER_EQUAL,
		TOKEN_OPER_NOT_EQUAL,
		TOKEN_OPER_LESS_EQUAL,
		TOKEN_OPER_MORE_EQUAL,
		TOKEN_OPER_LESS,
		TOKEN_OPER_MORE,
		TOKEN_OPER_OPEN_PAREN,
		TOKEN_OPER_CLOSE_PAREN
	};
	
	char char_oper[][OPER_CHAR_MAX] = {
		"+ ",
		"- ",
		"* ",
		"/ ",
		"~ ",
		"& ",
		"| ",
		"^ ",
		"! ",
		"<<",
		">>",
		"==",
		"!=",
		"<=",
		">=",
		"< ",
		"> ",
		"( ",
		") "
	};

	// add one in order for strlen() to not overflow
	char oper_buf[OPER_CHAR_MAX + 1] = { 0 };
	for(int i = 0; i < OPER_CHAR_MAX; i++) {
		oper_buf[i] = get_char();
		if(oper_buf[i] == '\n' || oper_buf[i] == '\r') {
			*oper = TOKEN_OPER_STOP;
			return;
		}
	}

	int max_oper = -1;
	*oper = TOKEN_OPER_STOP;
	for(int i = 0; i < ARRAY_SIZE(char_oper); i++) {
		char *oper_str = char_oper[i];
		int first_space = str_first_space(oper_str);

		// not just a space because str_first_space will stop
		// at 0 as well
		int temp_val_str = oper_str[first_space];
		int temp_val_buf = oper_buf[first_space];
		oper_str[first_space] = 0;
		oper_buf[first_space] = 0;
		if(
			strcmp(oper_str, oper_buf) == 0 &&
			first_space > max_oper
		) {
			max_oper = first_space;
			*oper = char_to_oper[i];
		}

		oper_buf[first_space] = temp_val_buf;
		oper_str[first_space] = temp_val_str;
	}

	rewind_stdin(OPER_CHAR_MAX - max_oper - 1, oper_buf);
	if(max_oper == -1) {
		*oper = TOKEN_OPER_STOP;
	}
}

void skip_whitespace() {
	char test_char = get_char();
	while(
		(test_char == ' ' || test_char == '\t') &&
		test_char != EOF
	) {
		test_char = get_char();
	}
	
	if(
		test_char != ' ' && 
		test_char != '\t' &&
		test_char != EOF
	) {
		rewind_stdin(1, &test_char);
	}
}

token_t fetch_token() {
	skip_whitespace();
	token_t token = { 0 };
	char test_char = get_char();
	if(
		test_char == '\n' || 
		test_char == '\r' || 
		test_char == EOF
	) {
		token.type = TOKEN_TYPE_OPER;
		token.data.oper = TOKEN_OPER_STOP;
		skip_whitespace();
		return token;
	}

	rewind_stdin(1, &test_char);
	if(test_char >= '0' && test_char <= '9') {
		token.type = TOKEN_TYPE_NUM;
		fetch_float(&token.data.num);
		skip_whitespace();
		return token;
	}

	token.type = TOKEN_TYPE_OPER;
	fetch_oper(&token.data.oper);
	skip_whitespace();
	return token;
}

float uniary_oper();
float binary_oper(float first_param) {
	token_t token = fetch_token();
	if(token.type != TOKEN_TYPE_OPER) {
		printf("coudn't find a operator, found another number instead\n");
		exit(1);
		return 0;
	}

	token_oper_e oper = token.data.oper;
	switch(oper) {
		case TOKEN_OPER_ADD: return first_param + uniary_oper();
		case TOKEN_OPER_SUB: return first_param - uniary_oper();
		case TOKEN_OPER_MUL: return first_param * uniary_oper();
		case TOKEN_OPER_DIV: return first_param / uniary_oper();
		case TOKEN_OPER_AND: return (int)(first_param) & (int)(uniary_oper());
		case TOKEN_OPER_OR: return (int)(first_param) | (int)(uniary_oper());
		case TOKEN_OPER_XOR: return (int)(first_param) ^ (int)(uniary_oper());
		case TOKEN_OPER_SHIFT_LEFT: return (int)(first_param) << (int)(uniary_oper());
		case TOKEN_OPER_SHIFT_RIGHT: return (int)(first_param) >> (int)(uniary_oper());
		case TOKEN_OPER_EQUAL: return first_param == uniary_oper();
		case TOKEN_OPER_NOT_EQUAL: return first_param != uniary_oper();
		case TOKEN_OPER_LESS_EQUAL: return first_param <= uniary_oper();
		case TOKEN_OPER_MORE_EQUAL: return first_param >= uniary_oper();
		case TOKEN_OPER_LESS: return first_param < uniary_oper();
		case TOKEN_OPER_MORE: return first_param > uniary_oper();
		case TOKEN_OPER_STOP: return first_param;
		default: printf("couldn't find binary operator after number\n"); exit(1); break;
	}
}

float uniary_oper() {
	token_t token = fetch_token();
	switch(token.type) {
		case TOKEN_TYPE_NUM: {
			float first_param = token.data.num;
			return binary_oper(first_param);
		}

		case TOKEN_TYPE_OPER: {
			token_oper_e oper = token.data.oper;
			switch(oper) {
				case TOKEN_OPER_ADD: return uniary_oper();
				case TOKEN_OPER_SUB: return -uniary_oper();
				case TOKEN_OPER_NOT: return ~(int)(uniary_oper());
				case TOKEN_OPER_BOOL_NOT: return !(int)(uniary_oper());
				case TOKEN_OPER_OPEN_PAREN: {
					float result = uniary_oper();
					token_t close_token = fetch_token();
					if(
						close_token.type != TOKEN_TYPE_OPER && 
						close_token.data.oper != TOKEN_OPER_CLOSE_PAREN
					) {
						printf("couldn't find closing parenthesis\n");
						exit(1);
						break;
					}

					return result;
				}

				case TOKEN_OPER_STOP: return 0;
				default: printf("couldn't find number or uniary operator\n"); exit(1); break;
			}
		}
	}
}

void setup_term() {
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);
	int desc = fileno(stdin);
	int flags = fcntl(desc, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(desc, F_SETFL, flags);
}

int main() {
	setup_term();
	for(;;) {
		is_last_newline = 0;
		float result = uniary_oper();
		printf("result: %f\n\n", result);
	}
	
	return 0;
}