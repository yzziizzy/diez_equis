#ifndef __diez_equis__str_utils_h__
#define __diez_equis__str_utils_h__




long strp_len(char** sp);
long strp_total_len(char** sp);




#define path_join(...) path_join_(PP_NARG(__VA_ARGS__), __VA_ARGS__)
char* path_join_(size_t nargs, ...);

#define strjoin(j, ...) strjoin_(j, PP_NARG(__VA_ARGS__), __VA_ARGS__)
char* strjoin_(char* joiner, size_t nargs, ...);

#define strcatdup(...) strcatdup_(PP_NARG(__VA_ARGS__), __VA_ARGS__)
char* strcatdup_(size_t nargs, ...);

char* sprintfdup(char* fmt, ...);

char** read_split_file(char* path, char* sep);


typedef struct {
	long len;
	long alloc;
} string_prefix_t;


#define str_len(s) (((string_prefix_t*)(s))->len)
#define str_alloc(s) (((string_prefix_t*)(s))->alloc)

char* str_new(long alloc);
void str_free(char* s);
char* str_concat(char* a, char* b);


typedef struct {
	long len;
	long alloc;
} string_list_prefix_t;

#define strp_len(sp) (( ((string_list_prefix_t*)(sp)) - 1)->len)
#define strp_alloc(sp) (( ((string_list_prefix_t*)(sp)) - 1)->alloc)

char** strp_new(long alloc);
void strp_free(char** s);
char** strp_concat(char** a, char** b);
char* strp_join(char** sp, char* joiner);


// returns a magic strp
char** strp_split(char* in, char* splitters);

// manually calculate list length looking for the null
long strp_clen(char** list);

void strp_push(char** sp, char* s);
void strp_pushndup(char** sp, char* s, long n);


void strp_print(char** sp, char* fmt);


typedef unsigned long hash_t;

hash_t strhash(char* str);
hash_t strnhash(char* str, size_t n);


long strprefix(char* s, char* prefix);

// looks for regex inside /slashes/ 
// returns the initial length of the regex
long strrecognizeregex(char* s);

// looks for C-style quoted strings 
// returns the initial length of the string, including quotes
long strrecognizestring(char* s);


// libPCRE wrappers:

int str_test(char* subject, char* pattern, char* opts);
char** str_match_all(char* subject, char* pattern, char* opts);


typedef struct {
	pcre2_code* code;
	pcre2_match_data* match;
} re_t;

re_t* re_compile(char* pattern, char* opts);
void re_free(re_t* re);
int re_test(re_t* re, char* subject);
char* re_match_one(re_t* re, char* subject);


#endif
