#ifndef __diez_equis__parser_h__
#define __diez_equis__parser_h__







void parser_test(char* input);





enum {
	NODE_RE, // performs regex-based matching
	NODE_STR,
	NODE_ENUM,
	NODE_SEQ,
	NODE_PLUS,
	NODE_STAR,
	NODE_ANY,
	NODE_OPTIONAL,
	NODE_NL,
	NODE_REGEX_MATCHER, // identifies a string that is a /slashed/ regex expression
	NODE_STRING_MATCHER, // identifies a double-quoted C-style string
};

typedef struct node {
	int type;
	char* name;
	
	union {
		re_t* re;
		char* str;
		struct node* n;
		VEC(struct node*) list;
		VEC(char*) strings;
	};
	
} node_t;


node_t* mk_re(char* name, char* pattern, char* opts);
node_t* mk_str(char* s);
node_t* mk_enum(char* s);

#define mk_seq(name, ...) mk_seq_(name, PP_NARG(__VA_ARGS__), __VA_ARGS__)
node_t* mk_seq_(char* name, int nargs, ...);

void list_push(node_t* list, node_t* item);
void strings_push(node_t* list, char* item);
node_t* mk_any();
node_t* mk_plus(char* name, node_t* child);
node_t* mk_star(char* name, node_t* child);
node_t* mk_optional(node_t* child);
node_t* mk_nl();
node_t* mk_regex_matcher(char* name);
node_t* mk_string_matcher(char* name);


enum {
	AST_ITEM,
	AST_LIST,
};

typedef struct ast {
	int type;
	char* name;
	
	char* text;
	VEC(struct ast*) kids;
} ast_t;



ast_t* mk_ast(char* type, char* name, char* text, size_t text_len);
void print_ast(ast_t* a);
void free_ast(ast_t* a);


#define EAT_WS 0x0001
ast_t* probe(node_t* n, char* input, int* offset, unsigned long opts);




#endif // __diez_equis__parser_h__
