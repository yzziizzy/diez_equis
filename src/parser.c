#include "inc.h"






/*

ident = /[_a-z][_a-z0-9]* /i

ws = /[\s]+/
ows = /[\s]* /

decl = ?ws ident ws ident ?ws ";" ?ws

decl_list = +decl

struct_decl = "struct" ?ws "{" decl_list "}"

*/






#define EMPTY_SUCCESS ((ast_t*)1)


char* grammar =
"%eat_ws\n"

"ident = /[_a-z][_a-z0-9]*/ \n"
"ws = /[\\s]+/ \n"

"exp = ident\n"
"exp = \"?\" ident\n"
"exp = \"+\" ident\n"
"exp = \":\" ident\n"
"exp = \"[\" +exp \"]\"\n"


"rule = name \"=\" +exp :nl\n"
"option = \"%\" ident :nl\n"
;



/*

typedef struct bar {
	float length; // = 0.5, clamp <0,1>
	struct bar* next;
} bar_t;

typedef struct foo {
	int prop; // = 4
	bar_t* items; // [next]
	long n_items; // #items

} foo_t;

*/

void parser_test(char* input) {
	node_t* n_ident = mk_re("ident", "^[_a-z][_a-z0-9]*", "i");

	node_t* n_special_fn = mk_enum("special");
	strings_push(n_special_fn, "string_matcher"); 
	strings_push(n_special_fn, "regex_matcher");
	strings_push(n_special_fn, "nl");

	node_t* n_exp = mk_any();
	
	node_t* n_exp_q = mk_seq("exp_?", mk_str("?"), n_ident);
	node_t* n_exp_p = mk_seq("exp_+", mk_str("+"), n_ident);
	node_t* n_exp_c = mk_seq("exp_:", mk_str(":"), n_special_fn);
	node_t* n_exp_l = mk_seq("exp_[]", mk_str("["), n_exp, mk_str("]"));
	
	list_push(n_exp, n_ident);  
	list_push(n_exp, n_exp_q);  
	list_push(n_exp, n_exp_p);  
	list_push(n_exp, n_exp_c);  
	list_push(n_exp, n_exp_l);  
	list_push(n_exp, mk_regex_matcher("regex"));  
	list_push(n_exp, mk_string_matcher("strings"));  
	
	node_t* n_option = mk_seq("option", mk_str("%"), n_ident, mk_nl());
	
	node_t* n_rule = mk_seq("rule", n_ident, mk_str("="), mk_plus("exp_list", n_exp), mk_nl());
	
	node_t* n_line = mk_any();
	list_push(n_line, n_rule);
	list_push(n_line, n_option);
	list_push(n_line, mk_nl());
	
	node_t* n_root = mk_star("lines", n_line);
	
	
	
	/*
	
	node_t* n_ws = mk_re("ws", "^\\s+", "i");
	node_t* n_ows = mk_optional(n_ws);
	
	node_t* n_struct = mk_str("struct");
	node_t* n_lbrace = mk_str("{");
	node_t* n_rbrace = mk_str("}");
	node_t* n_semi = mk_str(";");
	
	node_t* n_decl = mk_seq("decl");
//	list_push(n_decl, n_ows);
	list_push(n_decl, n_ident);
//	list_push(n_decl, n_ws);
	list_push(n_decl, n_ident);
//	list_push(n_decl, n_ows);
	list_push(n_decl, n_semi);
//	list_push(n_decl, n_ows);
	
	node_t* n_decl_list = mk_plus("decl_list", n_decl);
	
	node_t* n_struct_decl = mk_seq("struct_decl");
	list_push(n_struct_decl, n_struct);
//	list_push(n_struct_decl, n_ows);
	list_push(n_struct_decl, n_lbrace);
	list_push(n_struct_decl, n_decl_list);
	list_push(n_struct_decl, n_rbrace);
	*/
	
	int off = 0;
//	ast_t* a = probe(n_struct_decl, input, &off, EAT_WS);
	ast_t* a = probe(n_root, grammar, &off, EAT_WS);
	
	printf("\n\n");
	print_ast(a);
	
	
}


int indent = 0;
void ind() {
	for(int i = 0; i < indent; i++) printf("  ");
}


int is_flat_space(int c) {
	return (c == ' ' || c == '\t' || c == '\r' || c == '\v');
}


// NULL on failure
ast_t* probe(node_t* n, char* input, int* offset, unsigned long opts) {
	if(input[*offset] == 0) return NULL;
	
	indent++;
	
	ast_t* a;
	
	if(opts & EAT_WS) {
		while(is_flat_space(*(input + *offset))) *offset += 1;
	}
	
	#define return return indent--, 
	
	switch(n->type) {
		case NODE_NL: {
			ind(); printf("executing newline\n");
			if(*(input + *offset) == '\n') {
				*offset += 1;
				return EMPTY_SUCCESS;
			}
			
//			if(*(input + *offset) == 0) return EMPTY_SUCCESS;
			return NULL;
		}
		 
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
		
		case NODE_ENUM: {
			VEC_EACH(&n->strings, stri, str) {
				int span = strprefix(input + *offset, str);
				if(span) {
					ind(); printf("found string: '%.*s'\n", span, str);
					a = mk_ast(AST_ITEM, n->name, str, span);
					*offset += span;
					return a;
				}
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
				if(input[off] == '\0') return NULL;
				aa = probe(n->n, input, &off, opts);
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
				if(input[off] == '\0') return a;
				aa = probe(n->n, input, &off, opts);
				if(aa > EMPTY_SUCCESS) VEC_PUSH(&a->kids, aa);
				if(aa == NULL) break;
			} while(aa);
			
			*offset = off;
			
			return a;
		}
		
		case NODE_OPTIONAL: {
			a = probe(n->n, input, offset, opts);
			ind(); printf("executing optional\n");
			return a ? a : EMPTY_SUCCESS;
		}
		
		case NODE_SEQ: {
			int off = *offset;
			ast_t* aa;
			a = mk_ast(AST_LIST, n->name, NULL, 0);
			
			ind(); printf("executing sequence '%s'\n", n->name);
			VEC_EACH(&n->list, lni, ln) {
				aa = probe(ln, input, &off, opts);
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
				if(input[off] == '\0') return NULL;
				if(a = probe(ln, input, &off, opts)) {		
					*offset = off;
					return a;
				}
			}
			
			return NULL;
		}
		
		case NODE_REGEX_MATCHER: {
			ind(); printf("executing regex matcher\n");
			int span = strrecognizeregex(input + *offset);
			if(span) {
				a = mk_ast(AST_ITEM, n->name, input + *offset + 1, span - 2);
				*offset += span;
				return a;
			}
			
			return NULL;
		}
		
		case NODE_STRING_MATCHER: {
			ind(); printf("executing string matcher\n");
			int span = strrecognizestring(input + *offset);
			if(span) {
			span++;
				printf("found string '%.*s'\n", span, input + *offset);
				a = mk_ast(AST_ITEM, n->name, input + *offset + 1, span - 2);
				*offset += span;
				return a;
			}
			
			printf("no string\n");
			return NULL;
		}
		
	}
	
	return NULL;
	#undef return
}







node_t* mk_nl() {
	new(node_t*, n);

	n->type = NODE_NL;
		
	return n;
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

node_t* mk_enum(char* s) {
	new(node_t*, n);

	n->type = NODE_ENUM;
	n->name = strdup(s);
		
	return n;
}

node_t* mk_seq_(char* name, int nargs, ...) {
	new(node_t*, n);

	n->type = NODE_SEQ;
	n->name = strdup(name);
	
	va_list va;
	va_start(va, nargs);
	
	for(size_t i = 0; i < nargs; i++) {
		node_t* c = va_arg(va, node_t*);
		list_push(n, c);
	}
	
	va_end(va);
	
	return n;
}


void list_push(node_t* list, node_t* item) {
	VEC_PUSH(&list->list, item);
}

void strings_push(node_t* list, char* item) {
	VEC_PUSH(&list->strings, item);
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

node_t* mk_string_matcher(char* name) {
	new(node_t*, n);

	n->type = NODE_STRING_MATCHER;
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


