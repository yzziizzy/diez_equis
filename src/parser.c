#include "inc.h"






/*

ident = /[_a-z][_a-z0-9]* /i

ws = /[\s]+/
ows = /[\s]* /

decl = ?ws ident ws ident ?ws ";" ?ws

decl_list += decl

struct_decl = "struct" ?ws "{" decl_list "}"

*/



enum {
	NODE_RE, // performs regex-based matching
	NODE_STR,
	NODE_SEQ,
	NODE_PLUS,
	NODE_STAR,
	NODE_ANY,
	NODE_OPTIONAL,
	NODE_REGEX_MATCHER, // identifies a string that is a /slashed/ regex expression
};

typedef struct node {
	int type;
	char* name;
	
	union {
		re_t* re;
		char* str;
		struct node* n;
		VEC(struct node*) list;
	};
	
	struct node* sibling;
	struct node* kids;
} node_t;


node_t* mk_re(char* name, char* pattern, char* opts);
node_t* mk_str(char* s);
node_t* mk_seq(char* name);
void list_push(node_t* list, node_t* item);
node_t* mk_plus(char* name, node_t* child);
node_t* mk_star(char* name, node_t* child);
node_t* mk_optional(node_t* child);
node_t* mk_regex_matcher(char* name);

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

ast_t* probe(node_t* n, char* input, int* offset);


#define EMPTY_SUCCESS ((ast_t*)1)

void parser_test(char* input) {
	
	node_t* n_ident = mk_re("ident", "^[_a-z][_a-z0-9]*", "i");
	node_t* n_ws = mk_re("ws", "^\\s+", "i");
	node_t* n_ows = mk_optional(n_ws);
	
	node_t* n_struct = mk_str("struct");
	node_t* n_lbrace = mk_str("{");
	node_t* n_rbrace = mk_str("}");
	node_t* n_semi = mk_str(";");
	
	node_t* n_decl = mk_seq("decl");
	list_push(n_decl, n_ows);
	list_push(n_decl, n_ident);
	list_push(n_decl, n_ws);
	list_push(n_decl, n_ident);
	list_push(n_decl, n_ows);
	list_push(n_decl, n_semi);
	list_push(n_decl, n_ows);
	
	node_t* n_decl_list = mk_plus("decl_list", n_decl);
	
	node_t* n_struct_decl = mk_seq("struct_decl");
	list_push(n_struct_decl, n_struct);
	list_push(n_struct_decl, n_ows);
	list_push(n_struct_decl, n_lbrace);
	list_push(n_struct_decl, n_decl_list);
	list_push(n_struct_decl, n_rbrace);
	
	
	int off = 0;
	ast_t* a = probe(n_struct_decl, input, &off);
	
	printf("\n\n");
	print_ast(a);
	
	
}


int indent = 0;
void ind() {
	for(int i = 0; i < indent; i++) printf("  ");
}




// NULL on failure
ast_t* probe(node_t* n, char* input, int* offset) {
	indent++;
	
	ast_t* a;
	
	#define return return indent--, 
	
	switch(n->type) {
		case NODE_STR: { 
			int span = strprefix(input + *offset, n->str);
			if(span) {
				ind(); printf("found string: '%.*s'\n", span, input + *offset);
				a = mk_ast(AST_ITEM, n->name, input + *offset, span);
				*offset += span;
				return a;
			}
				
			return NULL;
		}
		
		case NODE_RE: {
			char* s = re_match_one(n->re, input + *offset);
			if(s) {
				ind(); printf("found regex match: '%s'\n", s);
				a = mk_ast(AST_ITEM, n->name, input + *offset, strlen(s));
				*offset += strlen(s);
				free(s);
				return a;
			}
			
			// failure
			return NULL;
		}
		
		case NODE_PLUS: {
			int cnt = 0;
			int off = *offset;
			ast_t* aa;
			a = mk_ast(AST_LIST, n->name, NULL, 0);
			
			while(1) {
				aa = probe(n->n, input, &off);
				if(!aa) {
					break;
				}
				else {
					if(aa != EMPTY_SUCCESS) VEC_PUSH(&a->kids, aa);
				}
				
				cnt++;
			}
			
			if(cnt >= 1) {
				*offset = off;
				ind(); printf("found %d plus iterations\n", cnt);
				return a;
			}
			
			free_ast(a);
			return NULL;
		}
		
		case NODE_STAR: {
			int off = *offset;
			ast_t* aa = NULL;
			a = mk_ast(AST_LIST, n->name, NULL, 0);
			
			
			ind(); printf("executing star\n");
			do {
				aa = probe(n->n, input, &off);
				if(aa > EMPTY_SUCCESS) VEC_PUSH(&a->kids, aa);
			} while(aa);
			
			*offset = off;
			
			return a;
		}
		
		case NODE_OPTIONAL: {
			a = probe(n->n, input, offset);
			ind(); printf("executing optional\n");
			return a ? a : EMPTY_SUCCESS;
		}
		
		case NODE_SEQ: {
			int off = *offset;
			ast_t* aa;
			a = mk_ast(AST_LIST, n->name, NULL, 0);
			
			ind(); printf("executing sequence\n");
			VEC_EACH(&n->list, lni, ln) {
				aa = probe(ln, input, &off);
				if(aa < EMPTY_SUCCESS) {
					free_ast(a);
					return NULL;
				}
				
				if(aa > EMPTY_SUCCESS) VEC_PUSH(&a->kids, aa);
			}
			
			*offset = off;
			
			return a;
		}
		
		case NODE_ANY: {
			int off = *offset;
			
			ind(); printf("executing any\n");
			VEC_EACH(&n->list, lni, ln) {
				if(a = probe(ln, input, &off)) {		
					*offset = off;
					return a;
				}
			}
			
			return NULL;
		}
		
		case NODE_REGEX_MATCHER: {
		
			int span = strrecognizeregex(input + *offset);
			if(span) {
				a = mk_ast(AST_ITEM, n->name, input + *offset + 1, span - 1);
				*offset += span;
				return a;
			}
			
			return NULL;
		}
		
	}
	
	return NULL;
	#undef return
}






node_t* mk_re(char* name, char* pattern, char* opts) {
	new(node_t*, n);

	n->type = NODE_RE;
	n->name = strdup(name);
	n->re = re_compile(pattern, opts);
	
	return n;
}


node_t* mk_str(char* s) {
	new(node_t*, n);

	n->type = NODE_STR;
	n->name = strdup(s);
	n->str = strdup(s);
		
	return n;
}

node_t* mk_seq(char* name) {
	new(node_t*, n);

	n->type = NODE_SEQ;
	n->name = strdup(name);
	
	return n;
}


void list_push(node_t* list, node_t* item) {
	VEC_PUSH(&list->list, item);
}


node_t* mk_plus(char* name, node_t* child) {
	new(node_t*, n);

	n->type = NODE_PLUS;
	n->name = strdup(name);
	n->n = child;
	
	return n;
}

node_t* mk_star(char* name, node_t* child) {
	new(node_t*, n);

	n->type = NODE_STAR;
	n->name = strdup(name);
	n->n = child;
	
	return n;
}

node_t* mk_any() {
	new(node_t*, n);
	
	n->type = NODE_ANY;
	
	return n;
}

node_t* mk_optional(node_t* child) {
	new(node_t*, n);

	n->type = NODE_OPTIONAL;
	n->n = child;
	
	return n;
}


node_t* mk_regex_matcher(char* name) {
	new(node_t*, n);

	n->type = NODE_REGEX_MATCHER;
	n->name = strdup(name);
	
	return n;
}



ast_t* mk_ast(char* type, char* name, char* text, size_t text_len) {
	new(ast_t*, a);
	
	a->type = type;
	a->name = strdup(name);
	a->text = strndup(text, text_len);
	
	return a;
}

void free_ast(ast_t* a) {
	VEC_EACH(&a->kids, i, aa) free_ast(aa);
	VEC_FREE(&a->kids);
	free(a);
}




void print_ast(ast_t* a) {
	
	ind(); printf("%s [%s]\n", a->name, a->text);
	
	indent++;
	VEC_EACH(&a->kids, i, aa) {
		print_ast(aa);
	}
	indent--;
}


