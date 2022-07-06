

#include "inc.h"


char* str_new(long alloc) {
	char* s = malloc(alloc + 1 + sizeof(string_prefix_t));
	
	string_prefix_t* sp = (string_prefix_t*)s;
	sp->len = 0;
	sp->alloc = alloc;
	
	return (char*)(sp + 1);
}

void str_free(char* s) {
	s -= sizeof(string_prefix_t);
	free(s);
}


char* str_concat(char* a, char* b) {
	long lena = strlen(a);
	long lenb = strlen(b);
	long len = lena + lenb;
	char* s = str_new(len);
	
	strncat(s, a, lena); 
	strncat(s + lena, b, lenb);
	s[len] = 0;
	str_len(s) = len;
	
	return s;
}




char** strp_new(long alloc) {
	char** sp = malloc(sizeof(*sp) * (alloc + 1) + sizeof(string_list_prefix_t));
	
	string_list_prefix_t* slp = (string_list_prefix_t*)sp;
	slp->len = 0;
	slp->alloc = alloc;
	
	return (char**)(slp + 1);
}

void strp_free(char** sp) {
	char* s = (char*)sp;
	s -= sizeof(string_list_prefix_t);
	free(s);
}



char** strp_concat(char** a, char** b) {
	long len = strp_len(a) + strp_len(b);
	char** s = strp_new(len);
	
	memcpy(s, a, strp_len(a) * sizeof(*a)); 
	memcpy(s + strp_len(a), b, strp_len(b) * sizeof(*a));
	s[len] = 0;
	strp_len(s) = strp_len(a) + strp_len(b);
	
	return s;
}


char* strp_join(char** sp, char* joiner) {
	
	return NULL;
}



void strp_check_alloc(char** sp, long extra) {
	if(strp_len(sp) + extra + 1 >= strp_alloc(sp)) {
		strp_alloc(sp) *= 2;
		sp = realloc(sp, strp_alloc(sp) * sizeof(*sp) + sizeof(string_list_prefix_t));
	}
}

void strp_push(char** sp, char* s) {
	strp_check_alloc(sp, 1);
	sp[strp_len(sp)] = s;
	strp_len(sp)++;
	sp[strp_len(sp)] = NULL;
}

void strp_pushndup(char** sp, char* s, long n) {
	strp_check_alloc(sp, 1);
	sp[strp_len(sp)] = strndup(s, n);
	strp_len(sp)++;
	sp[strp_len(sp)] = NULL;
}



char** strp_split(char* in, char* splitters) {
	char* e;
	char** list = strp_new(32); 
	
	for(char* s = in; *s;) {
		e = strpbrk(s, splitters);
		if(!e) e = s + strlen(s);
		
		
		strp_push(list, strndup(s, e - s));
		
		e += strspn(e, splitters);
		s = e;
	}
	
	return list;
}





hash_t strhash(char* str) {
	unsigned long h = 0;
	int c;

	while(c = *str++) {
		h = c + (h << 6) + (h << 16) - h;
	}
	return h;
}
hash_t strnhash(char* str, size_t n) {
	unsigned long h = 0;
	int c;

	while((c = *str++) && n--) {
		h = c + (h << 6) + (h << 16) - h;
	}
		
	return h;
}



long strp_clen(char** list) {
	long total = 0;
	for(; *list; list++) total++;
	return total;
}



void strp_print(char** sp, char* fmt) {
	for(char** s = sp; *sp; sp++) {
		printf(fmt, *sp);
	}	
}






char** concat_lists_(int nargs, ...) {
	size_t total = 0;
	char** out, **end;

	if(nargs == 0) return NULL;

	// calculate total list length
	va_list va;
	va_start(va, nargs);

	for(size_t i = 0; i < nargs; i++) {
		char** s = va_arg(va, char**);
		if(s) total += strp_clen(s);
	}

	va_end(va);

	out = malloc((total + 1) * sizeof(char**));
	end = out;

	va_start(va, nargs);
	
	// concat lists
	for(size_t i = 0; i < nargs; i++) {
		char** s = va_arg(va, char**);
		size_t l = strp_clen(s);
		
		if(s) {
			memcpy(end, s, l * sizeof(*s));
			end += l;
		}
	}

	va_end(va);

	*end = 0;

	return out;
}


char* join_str_list(char* list[], char* joiner) {
	size_t list_len = 0;
	size_t total = 0;
	size_t jlen = strlen(joiner);
	
	// calculate total length
	for(int i = 0; list[i]; i++) {
		list_len++;
		total += strlen(list[i]);
	}
	
	if(total == 0) return strdup("");
	
	total += (list_len - 1) * jlen;
	char* out = malloc((total + 1) * sizeof(*out));
	
	char* end = out;
	for(int i = 0; list[i]; i++) {
		char* s = list[i];
		size_t l = strlen(s);
		
		if(i > 0) {
			memcpy(end, joiner, jlen);
			end += jlen;
		}
		
		if(s) {
			memcpy(end, s, l);
			end += l;
		}
		
		total += strlen(list[i]);
	}
	
	*end = 0;
	
	return out;
}



#ifndef HAVE_STI
// concatenate all argument strings together in a new buffer
char* strcatdup_(size_t nargs, ...) {
	size_t total = 0;
	char* out, *end;
	
	if(nargs == 0) return NULL;
	
	// calculate total buffer len
	va_list va;
	va_start(va, nargs);
	
	for(size_t i = 0; i < nargs; i++) {
		char* s = va_arg(va, char*);
		if(s) total += strlen(s);
	}
	
	va_end(va);
	
	out = malloc((total + 1) * sizeof(char*));
	end = out;
	
	va_start(va, nargs);
	
	for(size_t i = 0; i < nargs; i++) {
		char* s = va_arg(va, char*);
		if(s) {
			strcpy(end, s); // not exactly the ost efficient, but maybe faster than
			end += strlen(s); // a C version. TODO: test the speed
		};
	}
	
	va_end(va);
	
	*end = 0;
	
	return out;
}
#endif

#ifndef HAVE_STI
// concatenate all argument strings together in a new buffer,
//    with the given joining string between them
char* strjoin_(char* joiner, size_t nargs, ...) {
	size_t total = 0;
	char* out, *end;
	size_t j_len;
	
	if(nargs == 0) return NULL;
	
	// calculate total buffer len
	va_list va;
	va_start(va, nargs);
	
	for(size_t i = 0; i < nargs; i++) {
		char* s = va_arg(va, char*);
		if(s) total += strlen(s);
	}
	
	va_end(va);
	
	j_len = strlen(joiner);
	total += j_len * (nargs - 1);
	
	out = malloc((total + 1) * sizeof(char*));
	end = out;
	
	va_start(va, nargs);
	
	for(size_t i = 0; i < nargs; i++) {
		char* s = va_arg(va, char*);
		if(s) {
			if(i > 0) {
				strcpy(end, joiner);
				end += j_len;
			}
			
			strcpy(end, s); // not exactly the most efficient, but maybe faster than
			end += strlen(s); // a C version. TODO: test the speed
		};
	}
	
	va_end(va);
	
	*end = 0;
	
	return out;
}
#endif

// returns the length of prefix if s contains all of prefix as a prefix, otherwise 0
long strprefix(char* s, char* prefix) {
	int i;
	
	for(i = 0; *prefix; i++, prefix++, s++) {
		if(*prefix != *s) return 0;
	}

	return i;
}

// looks for regex inside /slashes/ 
// returns the initial length of the regex
long strrecognizeregex(char* s) {
	if(*s != '/') return 0;
	
	char* os = s; // save the original pointer
	
	int parens = 0;
	int in_brackets = 0;
	
	while(1) {
		if(*s == '\\') { 
			s++;
			if(*s == 0) goto ESCAPED_NULL;
			s++;
			continue; 
		}
		
		if(in_brackets) {
			if(*s == 0) goto UNBALANCED_BRACKET;
			if(*s == ']') in_brackets = 0;
			s++; 
			continue;
		}
		else {
			if(*s == 0) {
				if(parens == 0) goto MISSING_TRAILING_SLASH;
				else goto UNBALANCED_PAREN;
			}
			
			if(parens == 0 && *s == '/') break;
		
			if(*s == '(') parens++;
			if(*s == ')') parens--;
			s++;
			continue;
		}
	}
	
	return s - os;
	
ESCAPED_NULL:
	fprintf(stderr, "Escaped NULL in regular expression\n");
	return 0;
	
UNBALANCED_BRACKET:
	fprintf(stderr, "Unbalanced bracket in regular expression\n");
	return 0;
	
MISSING_TRAILING_SLASH:
	fprintf(stderr, "Missing trailing slash in regular expression\n");
	return 0;
	
UNBALANCED_PAREN:
	fprintf(stderr, "Unbalanced parenthesis in regular expression\n");
	return 0;
}


#ifndef HAVE_STI
// allocates a new buffer and calls sprintf with it
// why isn't this a standard function?
char* sprintfdup(char* fmt, ...) {
	va_list va;
	
	va_start(va, fmt);
	size_t n = vsnprintf(NULL, 0, fmt, va);
	char* buf = malloc(n + 1);
	va_end(va);
	
	va_start(va, fmt);
	vsnprintf(buf, n + 1, fmt, va);
	va_end(va);
	
	return buf;
}
#endif




char** read_split_file(char* path, char* sep) {
	int fd;
	struct stat st;
	char* contents;
	
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Could not open file '%s'.\n", path);
		return NULL;
	}
	
	fstat(fd, &st);
	
	contents = mmap(NULL, st.st_size + 1, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if(contents == MAP_FAILED) {
		fprintf(stderr, "Failed to map anonymous memory region of size %ld\n", st.st_size + 1);
	}
	
	contents = mmap(contents, st.st_size, PROT_READ, MAP_SHARED | MAP_FIXED | MAP_POPULATE, fd, 0);
	if(contents == MAP_FAILED) {
		fprintf(stderr, "Failed to map file into memory: '%s'\n", path);
	}
	
	char** out = strp_split(sep, contents);
	
	munmap(contents, st.st_size);
	close(fd);

	return out;
}





int str_test(char* subject, char* pattern, char* opts) {
	pcre2_code* re;

	int errno;
	PCRE2_SIZE erroff;
	PCRE2_UCHAR errbuf[256];
	pcre2_match_data* match;
	
	uint32_t options = 0;
	
	for(; *opts; opts++) {
		if(*opts == 'i') options |= PCRE2_CASELESS;
	}
	
//	if(find_opt->match_mode == GFMM_PLAIN) {
//		options |= PCRE2_LITERAL;
//	}
	
	re = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, options, &errno, &erroff, NULL);
	if(!re) {
		pcre2_get_error_message(errno, errbuf, sizeof(errbuf));
		fprintf(stderr, "PCRE find error #1: '%s' \n", errbuf);
		
		return NULL;
	}
	
	// compilation was successful.
	match = pcre2_match_data_create_from_pattern(re, NULL);
	
	uint32_t m_opts = PCRE2_NOTEMPTY | PCRE2_NOTEMPTY_ATSTART;
	int ret;
	int res;
	res = pcre2_match(re, subject, strlen(subject), 0, m_opts, match, NULL);
	
	if(res > 0) {
		// found a match
		ret = 0;
	}
	else {
		// no match
		
		if(res != PCRE2_ERROR_NOMATCH) {
			// real error of some sort	
			char errbuf[256];
			pcre2_get_error_message(res, errbuf, sizeof(errbuf));
			
			fprintf(stderr, "PCRE real error: %p %p '%s'\n", re, subject, errbuf);
			
			ret = -1;
		}
		
		ret = 1;
	}
	

	// clean up regex structures
	if(re) {
		pcre2_code_free(re);
		pcre2_match_data_free(match);
	}
		
	return ret;
}


char** str_match_all(char* subject, char* pattern, char* opts) {

	pcre2_code* re;
	pcre2_match_data* match;


	intptr_t findCharS;
	intptr_t findCharE;
	intptr_t findLen;
	char* findREError;
	int findREErrorChar;
	
	int base = 0;
	int end = strlen(subject);

	int errno;
	PCRE2_SIZE erroff;
	PCRE2_UCHAR errbuf[256];
	
	
	uint32_t options = 0;
	
	for(; *opts; opts++) {
		if(*opts == 'i') options |= PCRE2_CASELESS;
	}
	
	
	re = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, options, &errno, &erroff, NULL);
	if(!re) {
		pcre2_get_error_message(errno, errbuf, sizeof(errbuf));
		fprintf(stderr, "PCRE find error #1: '%s' \n", errbuf);
		
		return NULL;
	}
	
	// compilation was successful.
	
	match = pcre2_match_data_create_from_pattern(re, NULL);

	int off = 0; // this is for partial matches
	uint32_t m_opts = PCRE2_NOTEMPTY | PCRE2_NOTEMPTY_ATSTART;
	int res;
	int wraps = 0;
	
	char** results = strp_new(16);
	
	while(base < end) {
		res = pcre2_match(re, subject + base, strlen(subject + base), off, m_opts, match, NULL);
		
		if(res > 0) {
			// found a match
		
			PCRE2_SIZE* ovec = pcre2_get_ovector_pointer(match);
			
			strp_pushndup(results, subject + base + (int)ovec[0], (int)ovec[1] - (int)ovec[0]);
			base += (int)ovec[1];  
		}
		else {
			// no match
			
			if(res != PCRE2_ERROR_NOMATCH) {
				// real error of some sort	
				char errbuf[256];
				pcre2_get_error_message(res, errbuf, sizeof(errbuf));
				
				fprintf(stderr, "PCRE real error: %p %p '%s'\n", re, subject, errbuf);
				
				return NULL;
			}
			
			break;
		}
	}
	

	// clean up regex structures
	if(re) {
		pcre2_code_free(re);
		pcre2_match_data_free(match);
	}
		
	return results;
}



re_t* re_compile(char* pattern, char* opts) {
	
	pcre2_code* re;
	pcre2_match_data* match;				
	uint32_t options = 0;
	
	int errno;
	PCRE2_SIZE erroff;
	PCRE2_UCHAR errbuf[256];
	
	for(; *opts; opts++) {
		if(*opts == 'i') options |= PCRE2_CASELESS;
	}
	
	
	re = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, options, &errno, &erroff, NULL);
	if(!re) {
		pcre2_get_error_message(errno, errbuf, sizeof(errbuf));
		fprintf(stderr, "PCRE find error #1: '%s' \n", errbuf);
		
		return NULL;
	}
	
	// compilation was successful.
	
	match = pcre2_match_data_create_from_pattern(re, NULL);
	
	re_t* r = malloc(sizeof(*r));
	r->code = re;
	r->match = match;
	
	return r;
}


void re_free(re_t* re) {
	if(re) {
		pcre2_code_free(re->code);
		pcre2_match_data_free(re->match);
		free(re);
	}
}


int re_test(re_t* re, char* subject) {
	
	PCRE2_SIZE erroff;
	PCRE2_UCHAR errbuf[256];
	
	uint32_t m_opts = PCRE2_NOTEMPTY | PCRE2_NOTEMPTY_ATSTART;
	int ret;
	int res;
	res = pcre2_match(re->code, subject, strlen(subject), 0, m_opts, re->match, NULL);
	
	if(res > 0) {
		// found a match
		ret = 0;
	}
	else {
		// no match
		
		if(res != PCRE2_ERROR_NOMATCH) {
			// real error of some sort	
			char errbuf[256];
			pcre2_get_error_message(res, errbuf, sizeof(errbuf));
			
			fprintf(stderr, "PCRE real error: %p '%s'\n", subject, errbuf);
			
			ret = -1;
		}
		
		ret = 1;
	}
	
	return ret;
}



char* re_match_one(re_t* re, char* subject) {
	// compilation was successful.
	
	int off = 0; // this is for partial matches
	uint32_t m_opts = PCRE2_NOTEMPTY | PCRE2_NOTEMPTY_ATSTART;
	int res;
	int wraps = 0;
	
	char* ret = NULL;
	
	res = pcre2_match(re->code, subject, strlen(subject), off, m_opts, re->match, NULL);
	
	if(res > 0) {
		// found a match
	
		PCRE2_SIZE* ovec = pcre2_get_ovector_pointer(re->match);
		
		ret = strndup(subject + (int)ovec[0], (int)ovec[1] - (int)ovec[0]);
	}
	else {
		// no match
		
		if(res != PCRE2_ERROR_NOMATCH) {
			// real error of some sort	
			char errbuf[256];
			pcre2_get_error_message(res, errbuf, sizeof(errbuf));
			
			fprintf(stderr, "PCRE real error: %p '%s'\n", subject, errbuf);
			
			return NULL;
		}
		
	}

	return ret;
}




