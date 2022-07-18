
#include "inc.h"




/*

name = /^[-_a-z][-_a-z0-9 ]* ?/
anything = /^[^\\n]* ?/
distance = /^[1-9][0-9]*k?m/

ini_pair = name "=" anything :nl

place = "[place]" :nl *place_contents 
river = "[river]" :nl *place_contents 
place_contents = :nl
place_contents = ini_pair
place_contents = "contains" "{" :nl *contains_contents "}" :nl
place_contents = "constraints" "{" :nl *constraints_contents "}" :nl

contains_contents = :nl
contains_contents = ini_pair

constraints_contents = :nl
constraints_contents = ini_pair
constraints_contents[dist] = distance "<" name "<" distance :nl
constraints_contents[circa] = "near" name :nl
constraints_contents[circa] = "in" name :nl
constraints_contents[rel] = "south" ?"of" name :nl
constraints_contents[rel] = "north" ?"of" name :nl


# Option A: very ugly

struct constraint { // | constraints_contents
	int type; // = rule_type
	char* mindist; // = #1
	char* maxdist; // = #5
	char* rel; // #1
	char* other_place; // = dist:#3 circa:#2 rel:#3
	char* key, *val;
};

struct item { // | contains_contents
	char* key; // = #1
	char* val; // = #2
	struct item* next; // > next
};

struct place { // | place, river  
	char* name; // = rule_name
	struct item* items; // = ll contains
	VEC(struct constraint*) cons; // = vec constraints
};



# Option B: maybe crazy

ini_pair = name "=" anything :nl                            | &key _ &value

constraints_contents = :nl                                  |  !ignore
constraints_contents = ini_pair                             |  constraint.key=key constraint.val=value
constraints_contents = distance "<" name "<" distance :nl   | [constraint.type=dist] .mindist _ .other_place _ .maxdist 
constraints_contents = "near" name :nl                      | [constraint.type=circa] .rel .other_place
constraints_contents = "in" name :nl                        | [constraint.type=circa] .rel .other_place
constraints_contents = "south" ?"of" name :nl               | [constraint.type=rel] .rel _ .other_place
constraints_contents = "north" ?"of" name :nl               | [constraint.type=rel] .rel _ .other_place


# Option C: unreadable

constraints_contents[dist>constraint.type] = distance>.mindist "<" name>.other_place "<" distance>.maxdist :nl
constraints_contents[circa>.type] = "near">.rel name>.other_place :nl
constraints_contents[circa>.type] = "in">.rel name>.other_place :nl
constraints_contents[rel>.type] = "south">.rel ?"of" name>.other_place :nl
constraints_contents[rel>.type] = "north">.rel ?"of" name>.other_place :nl

*/




#define I(v, n) VEC_ITEM(&((v)->kids), n)
#define T(v, n) VEC_ITEM(&((v)->kids), n)->text

#define KV(prop) if(streq(#prop, T(line, 0))) { p->prop = T(line, 1); }

enum {
	CST_REL = 1,
	CST_RANGE,
};

enum {
	DIR_N = 1,
	DIR_NW,
	DIR_W,
	DIR_SW,
	DIR_S,
	DIR_SE,
	DIR_E,
	DIR_NE,
};

typedef struct place {
	char* name;
	char* type;
	
} place_t;

typedef struct constraint {
	int type;
	union {
		struct {
			int dir;
			char* other_place;
		} rel;
		
		struct {
			double min, max;
			char* other_place;
		} range;
	
	};
} constraint_t;

typedef struct constraint_set {
	VEC(constraint_t*) set;
} constraint_set_t;


int decode_dir(char* s) {
	if(streqi("north", s)) return DIR_N;
	if(streqi("south", s)) return DIR_S;
	if(streqi("east", s)) return DIR_E;
	if(streqi("west", s)) return DIR_W;
	if(streqi("northeast", s)) return DIR_NE;
	if(streqi("northwest", s)) return DIR_NW;
	if(streqi("southeast", s)) return DIR_SE;
	if(streqi("southwest", s)) return DIR_SW;
	return 0;
}

void place_fn(node_t* n, ast_t* a) {
	ast_t* lines = I(a, 1);
	
	new(place_t*, p);
	
	VEC_EACH(&lines->kids, li, line) {
		if(streq("ini_pair", line->name)) {
			KV(name);
			KV(type);
		}
	} 
}

void constraints_fn(node_t* n, ast_t* a) {
	ast_t* lines = I(a, 1);
	
	new(constraint_set_t*, cs);
	a->value.vp = cs;
	
	VEC_EACH(&lines->kids, li, line) {
		VEC_PUSH(&cs->set, line->value.vp);
	}
}

void constriant_fn(node_t* n, ast_t* a) {
	new(constraint_t*, c);
	a->value.vp = c;
	
	if(streq("rel", a->name)) {
		c->type = CST_REL;
		c->rel.dir = decode_dir(T(a, 0));
		c->rel.other_place = strdup(T(a, 1));
	}
	else if(streq("range", a->name)) {
		c->type = CST_RANGE;
		c->range.min = decode_dir(T(a, 0));
		c->range.max = decode_dir(T(a, 2));
		c->range.other_place = strdup(T(a, 1));
	}

}

void distance_fn(node_t* n, ast_t* a) {
	char* e = NULL;
	double mul = 1;
	
	double d = strtod(a->text, &e);
	if(*e == 'm') mul = .001;
	if(*e == 'c') mul = .01;
	if(*e == 'k') mul = 1000;
	
	a->value.d = d * mul;
}



void parse_map() {
	node_t* n_name = mk_re("name", "^[-_a-z][-_a-z0-9 ]*", "i");
	node_t* n_anything = mk_re("anything", "^[^\\n#]*", "i");
	node_t* n_distance = cvt(distance_fn, mk_re("distance", "^[1-9][0-9]*k?m", "i"));
	
	node_t* n_el = nop(mk_seq("end line", mk_optional(mk_seq("comment", mk_str("#"), mk_re("truly_anything", "^[^\\n]*", "i"))), mk_nl()));

	node_t* n_ini_pair = mk_seq("ini_pair", low(n_name), nop(mk_str("=")), n_anything, n_el);

	node_t* n_place_contents = mk_any();
	node_t* n_contains_contents = mk_any();
	node_t* n_constraints_contents = cvt(constriant_fn, mk_any());
	
	list_push(n_contains_contents, n_el);
	list_push(n_contains_contents, n_ini_pair);
	list_push(n_place_contents, n_el);
	list_push(n_place_contents, n_ini_pair);
	list_push(n_constraints_contents, n_el);
	list_push(n_constraints_contents, mk_seq("range", n_distance, nop(mk_str("<")), n_name, nop(mk_str("<")), n_distance, n_el));
	list_push(n_constraints_contents, mk_seq("rel", mk_str("near"), n_name, n_el));
	list_push(n_constraints_contents, mk_seq("rel", mk_str("in"), n_name, n_el));
	list_push(n_constraints_contents, mk_seq("rel", mk_str("south"), nop(mk_optional(mk_str("of"))), n_name, n_el));
	list_push(n_constraints_contents, mk_seq("rel", mk_str("north"), nop(mk_optional(mk_str("of"))), n_name, n_el));
	list_push(n_constraints_contents, n_ini_pair);
		
	
	
	list_push(n_place_contents, mk_seq("contains", nop(mk_str("contains")), nop(mk_str("{")), n_el, mk_star("contains_line", n_contains_contents), nop(mk_str("}")), n_el));
	list_push(n_place_contents, mk_seq("constraints", nop(mk_str("constraints")), nop(mk_str("{")), n_el, mk_star("constraints_line", n_constraints_contents), nop(mk_str("}")), n_el));

	node_t* n_place = cvt(place_fn, mk_seq("place", mk_str("[place]"), n_el, mk_star("place_line", n_place_contents)));
	node_t* n_river = mk_seq("river", mk_str("[river]"), n_el, mk_star("river_line", n_place_contents));

	node_t* n_open_line = mk_any();
	list_push(n_open_line, n_el);
	list_push(n_open_line, n_place);
	list_push(n_open_line, n_river);

	
	node_t* n_root = mk_star("lines", n_open_line);






	char* src = readWholeFile("map.txt", NULL);
	
	int off = 0;
//	ast_t* a = probe(n_struct_decl, input, &off, EAT_WS);
	ast_t* a = probe(n_root, src, &off, EAT_WS | TRIM_TEXT | COLLAPSE_TEXT_WS);
	free(src);
	
	print_ast(a);
		
} 








