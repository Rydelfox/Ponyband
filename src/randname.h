#ifndef RANDNAME_H
#define RANDNAME_H

/*
 * The different types of name randname.c can generate
 * which is also the number of sections in names.txt
 */
typedef enum
{
	RANDNAME_TOLKIEN = 1,
	RANDNAME_SCROLL,
	RANDNAME_EPONY_FIRST,   /* Special - RANDNAME_X_FIRST triggers a different naming mechanism */
	RANDNAME_EPONY_LAST,    /* Avoid using directly, but will have the same results as RANDNAME_X_FIRST */
	RANDNAME_UPONY_FIRST, 
	RANDNAME_UPONY_LAST, 
	RANDNAME_PPONY_FIRST,
	RANDNAME_PPONY_LAST,
	RANDNAME_BPONY_FIRST,
	RANDNAME_BPONY_LAST,
	RANDNAME_APONY,         /* Choses randomly from E, U, and P Pony */
	RANDNAME_BUFFALO,       /* Uses EPONY without the space */
	RANDNAME_DONKEY_M,
	RANDNAME_DONKEY_F,
	RANDNAME_HUMAN_M,
	RANDNAME_HUMAN_F,
	RANDNAME_ZEBRA_M,
	RANDNAME_ZEBRA_F,
	RANDNAME_DRAGON,
	RANDNAME_DDOG_M,
	RANDNAME_DDOG_F,
	RANDNAME_GRIFFON_M,
	RANDNAME_GRIFFON_F,
	
	
 
	/* End of type marker - not a valid name type */
	RANDNAME_NUM_TYPES 
} randname_type;

/*
 * Make a random name.
 */
extern size_t randname_make(randname_type name_type, size_t min, size_t max, char *word_buf, size_t buflen, const char ***wordlist);

#endif /* RANDNAME_H */
