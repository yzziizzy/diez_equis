#ifndef __diez_equis__parser_h__
#define __diez_equis__parser_h__







void parser_test(char* input);





enum {
	NODE_UNKNOWN, // for reserved names without a definition yet
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


#define NODE_CVT_INT64 ((void*)1)
#define NODE_CVT_UINT64 ((void*)2)
#define NODE_CVT_DOUBLE ((void*)3)
#define NODE_CVT_FLOAT ((void*)4)

struct node;
struct ast;

typedef void (*node_cvt_fn_t)(struct node*, struct ast*);


typedef struct node {
	int type;
	unsigned int opts;
	node_cvt_fn_t cvt;
	
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
	
	// converted value from node->cvt
	union {
		void* vp;
		uint64_t u;
		int64_t s;
		double d;
		float f;
		char* str;
	} value;
} ast_t;



ast_t* mk_ast(char* type, char* name, char* text, size_t text_len, unsigned int opts);
void print_ast(ast_t* a);
void free_ast(ast_t* a);


#define EAT_WS           0x0001
#define TRIM_TEXT        0x0002
#define COLLAPSE_TEXT_WS 0x0004
#define TEXT_TO_LOWER    0x0008
#define IGNORE_OUTPUT    0x1000
ast_t* probe(node_t* n, char* input, int* offset, unsigned long opts);




static inline node_t* nop(node_t* n) {
	n->opts |= IGNORE_OUTPUT;
	return n;
}

static inline node_t* low(node_t* n) {
	n->opts |= TEXT_TO_LOWER;
	return n;
}

static inline node_t* cvt(node_cvt_fn_t fn, node_t* n) {
	n->cvt = fn;
	return n;
}


//void dump_recognizer(ast_t* root);


#endif // __diez_equis__parser_h__
