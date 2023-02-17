
#include "../inc.h"






typedef struct rule_info {
	char* name;
	char* parent;
	char* init;
	int defs;
} rule_info_t;

typedef struct parser_data {

	HT(rule_info_t*) rules;
	VEC(char*) lines;

	int next_id;
} parser_data_t;


void add_line(ast_t* a, char* parent, parser_data_t* pd);

void dump_exp(ast_t* a, char* parent, parser_data_t* pd);



#define p(...) VEC_PUSH(&pd->lines, sprintfdup(__VA_ARGS__))
#define I(v, n) VEC_ITEM(&((v)->kids), n)

FILE* f;

void dump_recognizer(ast_t* root) {
	
	f = stdout; //fdopen(0, "w");
	
	
	new(parser_data_t*, pd);
	pd->next_id = 1;
	HT_init(&pd->rules, 256);
	
	fprintf(f, "node_t* n_root = mk_any();\n");
	
	
	VEC_EACH(&root->kids, ki, k) {
		add_line(k, "root", pd);
	}
	
	HT_EACH(&pd->rules, key, rule_info_t*, rule) {
		fprintf(f, "node_t* n_%s = %s;\n", rule->name, rule->init);
	}
	
	HT_EACH(&pd->rules, key, rule_info_t*, rule) {
		fprintf(f, "list_push(n_%s, n_%s);\n", rule->parent, rule->name);
	}
	
	
	
	VEC_EACH(&pd->lines, i, l) {
		fprintf(f, "%s\n", l);
	}
	

}





void add_line(ast_t* a, char* parent, parser_data_t* pd) {
	
	printf("line: %s\n", a->name);
	
	if(0 == strcmp(a->name, "option")) {
//		p("options |= %s;\n", VEC_ITEM(&a->kids, 1)->text);
	
		return;	
	}

	if(0 == strcmp(a->name, "rule")) {
//		p("node_t* n_%d = ", pd->next_id);
		
		
		ast_t* an = I(a, 0);
		ast_t* ak = I(a, 2);
		
		
		rule_info_t* ri;
		if(HT_get(&pd->rules, an->text, &ri)) {
			ri = calloc(1, sizeof(*ri));
			ri->name = strdup(an->text);
			ri->parent = strdup(parent);
			
			ri->init = sprintfdup("mk_seq(\"%s\")", an->text);
			
			HT_set(&pd->rules, an->text, ri);
		}
		else {
			if(ri->defs == 1) {
				// TODO: print out def
			
				free(ri->init);
				ri->init = strdup("mk_any()");
			}
		}
		ri->defs++;
		
		if(streq("exp_list", ak->name)) {
			
			
			VEC_EACH(&ak->kids, ki, k) {
				dump_exp(k, an->text, pd);
//				p("\n");
			}
		
		}

		
		
	
		return;
	}
	
//	if(a->type == AST_LIST) { 
//		VEC_EACH(&a->kids, ki, k) {
//			dump_line(k, f);
//		}
//	}
}

/*
rule_i* ensure_rule(char* name, parser_data_t* pd) {
	
	node_t* n;
	
	if(HT_get(&pd->rules, name, &n)) {
		return n;
	}
	
	n = calloc(1, sizeof(*n));
	n->type = NODE_UNKNOWN;
	n->name = strdup(name);
	
	HT_set(&pd->rules, name, n);
	
	return n;
}


void add_rule(ast_t* a, parser_data_t* pd) {
	char* name = I(a, 0)->text; 
	ast_t* list = I(a, 2); 
	
	
	node_t* r = ensure_rule(name, pd);
	
	node_t* val = add_exp(list, pd);
	
	// TODO: check for any-behavior
	r->n = val;
}


*/


void dump_exp(ast_t* a, char* parent, parser_data_t* pd) {
	
	if(streq("exp_list", a->text)) {
		new(node_t*, seq);	
		seq->type = NODE_SEQ;
	
		VEC_EACH(&a->kids, ki, k) {
//			list_push(seq, add_exp(k, pd));
		}
		
//		return seq;
	}
	
	if(streq("regex", a->name)) {
		
		// todo: escape
		p("node_t* n_%d = mk_regex(\"%s\");", pd->next_id, a->text);
		p("list_push(n_%s, n_%d);", parent, pd->next_id++);
		return;
	}
	
	if(streq("ident", a->name)) {
		p("list_push(n_%s, n_%s);", parent, a->text);
		return;
	}
	
	if(streq("strings", a->name)) {
		p("list_push(n_%s, mk_str(\"%s\"));", parent, a->text);
		
		return ;
	}
	
	if(streq("exp_+", a->name)) {
		p("list_push(n_%s, mk_plus(\"%s\", n_%s));", parent, a->text, I(a, 1)->text);
		
		return;
	}
	
	
	if(streq("+", a->text)) {
		new(node_t*, n);
	
		n->type = NODE_PLUS;
		return;
	}
	
	if(streq("*", a->text)) {
		new(node_t*, n);
	
		n->type = NODE_STAR;
		return;
	}
	

	
}




