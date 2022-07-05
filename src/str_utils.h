#ifndef __diez_equis__str_utils_h__
#define __diez_equis__str_utils_h__




long strp_len(char** sp);
long strp_total_len(char** sp);



#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, N, ...) N
#define PP_RSEQ_N() 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_NARG(...)  PP_NARG_(__VA_ARGS__, PP_RSEQ_N())

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

// these use libpcre
int str_test(char* subject, char* pattern, char* opts);
char** str_match_all(char* subject, char* pattern, char* opts);

typedef struct {
	pcre2_code* re;
} re_t;




#endif
