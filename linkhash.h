/*
 * $Id: linkhash.h,v 1.6 2006/01/30 23:07:57 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

/**
 * @file
 * @brief Internal methods for working with json_type_object objects.  Although
 *        this is exposed by the json_object_get_object() function and within the
 *        json_object_iter type, it is not recommended for direct use.
 */
#ifndef _json_c_linkhash_h_
#define _json_c_linkhash_h_

#include "json_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * golden prime used in hash functions
 */
#define LH_PRIME 0x9e370001UL

/**
 * The fraction of filled hash buckets until an insert will cause the table
 * to be resized.
 * This can range from just above 0 up to 1.0.
 */
#define LH_LOAD_FACTOR 0.66

/**
 * sentinel pointer value for empty slots
 */
#define LH_EMPTY (void *)-1

/**
 * sentinel pointer value for freed slots
 */
#define LH_FREED (void *)-2

/**
 * default string hash function
 */
#define JSON_C_STR_HASH_DFLT 0

/**
 * perl-like string hash function
 */
#define JSON_C_STR_HASH_PERLLIKE 1

/**
 * This function sets the hash function to be used for strings.
 * Must be one of the JSON_C_STR_HASH_* values.
 * @returns 0 - ok, -1 if parameter was invalid
 */
int json_global_set_string_hash(const int h);

struct lh_entry;

/**
 * callback function prototypes
 */
typedef void(lh_entry_free_fn)(struct lh_entry *e);
/**
 * callback function prototypes
 */
typedef unsigned long(lh_hash_fn)(const void *k);
/**
 * callback function prototypes
 */
typedef int(lh_equal_fn)(const void *k1, const void *k2);

/**
 * An entry in the hash table.  Outside of linkhash.c, treat this as opaque.
 */
struct lh_entry
{
	/**
	 * The key.
	 * @deprecated Use lh_entry_k() instead of accessing this directly.
	 */
	const void *k;
	/**
	 * A flag for users of linkhash to know whether or not they
	 * need to free k.
	 * @deprecated use lh_entry_k_is_constant() instead.
	 */
	int k_is_constant;
	/**
	 * The value.
	 * @deprecated Use lh_entry_v() instead of accessing this directly.
	 */
	const void *v;
	/**
	 * The next entry.
	 * @deprecated Use lh_entry_next() instead of accessing this directly.
	 */
	struct lh_entry *next;
	/**
	 * The previous entry.
	 * @deprecated Use lh_entry_prev() instead of accessing this directly.
	 */
	struct lh_entry *prev;
};

/**
 * The hash table structure.  Outside of linkhash.c, treat this as opaque.
 */
struct lh_table
{
	/**
	 * Size of our hash.
	 * @deprecated do not use outside of linkhash.c
	 */
	int size;
	/**
	 * Numbers of entries.
	 * @deprecated Use lh_table_length() instead.
	 */
	int count;

	/**
	 * The first entry.
	 * @deprecated Use lh_table_head() instead.
	 */
	struct lh_entry *head;

	/**
	 * The last entry.
	 * @deprecated Do not use, may be removed in a future release.
	 */
	struct lh_entry *tail;

	/**
	 * Internal storage of the actual table of entries.
	 * @deprecated do not use outside of linkhash.c
	 */
	struct lh_entry *table;

	/**
	 * A pointer to the function responsible for freeing an entry.
	 * @deprecated do not use outside of linkhash.c
	 */
	lh_entry_free_fn *free_fn;
	/**
	 * @deprecated do not use outside of linkhash.c
	 */
	lh_hash_fn *hash_fn;
	/**
	 * @deprecated do not use outside of linkhash.c
	 */
	lh_equal_fn *equal_fn;
};
typedef struct lh_table lh_table;

/**
 * Convenience list iterator.
 */
#define lh_foreach(table, entry) for (entry = lh_table_head(table); entry; entry = lh_entry_next(entry))

/**
 * lh_foreach_safe allows calling of deletion routine while iterating.
 *
 * @param table a struct lh_table * to iterate over
 * @param entry a struct lh_entry * variable to hold each element
 * @param tmp a struct lh_entry * variable to hold a temporary pointer to the next element
 */
#define lh_foreach_safe(table, entry, tmp) \
	for (entry = lh_table_head(table); entry && ((tmp = lh_entry_next(entry)) || 1); entry = tmp)

/**
 * Create a new linkhash table.
 *
 * @param size initial table size. The table is automatically resized
 * although this incurs a performance penalty.
 * @param free_fn callback function used to free memory for entries
 * when lh_table_free or lh_table_delete is called.
 * If NULL is provided, then memory for keys and values
 * must be freed by the caller.
 * @param hash_fn  function used to hash keys. 2 standard ones are defined:
 * lh_ptr_hash and lh_char_hash for hashing pointer values
 * and C strings respectively.
 * @param equal_fn comparison function to compare keys. 2 standard ones defined:
 * lh_ptr_hash and lh_char_hash for comparing pointer values
 * and C strings respectively.
 * @return On success, a pointer to the new linkhash table is returned.
 * 	On error, a null pointer is returned.
 */
extern struct lh_table *lh_table_new(int size, lh_entry_free_fn *free_fn, lh_hash_fn *hash_fn,
                                     lh_equal_fn *equal_fn);

/**
 * Convenience function to create a new linkhash table with char keys.
 *
 * @param size initial table size.
 * @param free_fn callback function used to free memory for entries.
 * @return On success, a pointer to the new linkhash table is returned.
 * 	On error, a null pointer is returned.
 */
extern struct lh_table *lh_kchar_table_new(int size, lh_entry_free_fn *free_fn);

/**
 * Convenience function to create a new linkhash table with ptr keys.
 *
 * @param size initial table size.
 * @param free_fn callback function used to free memory for entries.
 * @return On success, a pointer to the new linkhash table is returned.
 * 	On error, a null pointer is returned.
 */
extern struct lh_table *lh_kptr_table_new(int size, lh_entry_free_fn *free_fn);

/**
 * Free a linkhash table.
 *
 * If a lh_entry_free_fn callback free function was provided then it is
 * called for all entries in the table.
 *
 * @param t table to free.
 */
extern void lh_table_free(struct lh_table *t);

/**
 * Insert a record into the table.
 *
 * @param t the table to insert into.
 * @param k a pointer to the key to insert.
 * @param v a pointer to the value to insert.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, a negative value is returned.
 */
extern int lh_table_insert(struct lh_table *t, const void *k, const void *v);

/**
 * Insert a record into the table using a precalculated key hash.
 *
 * The hash h, which should be calculated with lh_get_hash() on k, is provided by
 *  the caller, to allow for optimization when multiple operations with the same
 *  key are known to be needed.
 *
 * @param t the table to insert into.
 * @param k a pointer to the key to insert.
 * @param v a pointer to the value to insert.
 * @param h hash value of the key to insert
 * @param opts if set to JSON_C_OBJECT_ADD_CONSTANT_KEY, sets lh_entry.k_is_constant
 *             so t's free function knows to avoid freeing the key.
 */
extern int lh_table_insert_w_hash(struct lh_table *t, const void *k, const void *v,
                                  const unsigned long h, const unsigned opts);

/**
 * Lookup a record in the table.
 *
 * @param t the table to lookup
 * @param k a pointer to the key to lookup
 * @return a pointer to the record structure of the value or NULL if it does not exist.
 */
extern struct lh_entry *lh_table_lookup_entry(struct lh_table *t, const void *k);

/**
 * Lookup a record in the table using a precalculated key hash.
 *
 * The hash h, which should be calculated with lh_get_hash() on k, is provided by
 *  the caller, to allow for optimization when multiple operations with the same
 *  key are known to be needed.
 *
 * @param t the table to lookup
 * @param k a pointer to the key to lookup
 * @param h hash value of the key to lookup
 * @return a pointer to the record structure of the value or NULL if it does not exist.
 */
extern struct lh_entry *lh_table_lookup_entry_w_hash(struct lh_table *t, const void *k,
                                                     const unsigned long h);

/**
 * Lookup a record in the table.
 *
 * @param t the table to lookup
 * @param k a pointer to the key to lookup
 * @param v a pointer to a where to store the found value (set to NULL if it doesn't exist).
 * @return whether or not the key was found
 */
extern json_bool lh_table_lookup_ex(struct lh_table *t, const void *k, void **v);

/**
 * Delete a record from the table.
 *
 * If a callback free function is provided then it is called for the
 * for the item being deleted.
 * @param t the table to delete from.
 * @param e a pointer to the entry to delete.
 * @return 0 if the item was deleted.
 * @return -1 if it was not found.
 */
extern int lh_table_delete_entry(struct lh_table *t, struct lh_entry *e);

/**
 * Delete a record from the table.
 *
 * If a callback free function is provided then it is called for the
 * for the item being deleted.
 * @param t the table to delete from.
 * @param k a pointer to the key to delete.
 * @return 0 if the item was deleted.
 * @return -1 if it was not found.
 */
extern int lh_table_delete(struct lh_table *t, const void *k);

/**
 * Return the number of entries in the table.
 */
extern int lh_table_length(struct lh_table *t);

/**
 * Resizes the specified table.
 *
 * @param t Pointer to table to resize.
 * @param new_size New table size. Must be positive.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, a negative value is returned.
 */
int lh_table_resize(struct lh_table *t, int new_size);

/**
 * @deprecated Don't use this outside of linkhash.h:
 */
#if !defined (__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)
/* C89 compilers like VS2010 can't handle inline funcs, so skip it there,
   note: this also applies to -std=c89 in GCC! */
#define _LH_INLINE
#else
#define _LH_INLINE inline
#endif

/**
 * Return the first entry in the lh_table.
 * @see lh_entry_next()
 */
static _LH_INLINE struct lh_entry *lh_table_head(const lh_table *t)
{
	return t->head;
}

/**
 * Calculate the hash of a key for a given table.
 *
 * This is an extension to support functions that need to calculate
 * the hash several times and allows them to do it just once and then pass
 * in the hash to all utility functions. Depending on use case, this can be a
 * considerable performance improvement.
 * @param t the table (used to obtain hash function)
 * @param k a pointer to the key to lookup
 * @return the key's hash
 */
static _LH_INLINE unsigned long lh_get_hash(const struct lh_table *t, const void *k)
{
	return t->hash_fn(k);
}


/**
 * @deprecated Don't use this outside of linkhash.h:
 */
#ifdef __UNCONST
#define _LH_UNCONST(a) __UNCONST(a)
#else
#define _LH_UNCONST(a) ((void *)(uintptr_t)(const void *)(a))
#endif

/**
 * Return a non-const version of lh_entry.k.
 *
 * lh_entry.k is const to indicate and help ensure that linkhash itself doesn't modify
 * it, but callers are allowed to do what they want with it.
 * @see lh_entry_k_is_constant()
 */
static _LH_INLINE void *lh_entry_k(const struct lh_entry *e)
{
	return _LH_UNCONST(e->k);
}

/**
 * Returns 1 if the key for the given entry is constant, and thus
 *  does not need to be freed when the lh_entry is freed.
 * @see lh_table_insert_w_hash()
 */
static _LH_INLINE int lh_entry_k_is_constant(const struct lh_entry *e)
{
	return e->k_is_constant;
}

/**
 * Return a non-const version of lh_entry.v.
 *
 * v is const to indicate and help ensure that linkhash itself doesn't modify
 * it, but callers are allowed to do what they want with it.
 */
static _LH_INLINE void *lh_entry_v(const struct lh_entry *e)
{
	return _LH_UNCONST(e->v);
}

/**
 * Change the value for an entry.  The caller is responsible for freeing
 *  the previous value.
 */
static _LH_INLINE void lh_entry_set_val(struct lh_entry *e, void *newval)
{
	e->v = newval;
}

/**
 * Return the next element, or NULL if there is no next element.
 * @see lh_table_head()
 * @see lh_entry_prev()
 */
static _LH_INLINE struct lh_entry *lh_entry_next(const struct lh_entry *e)
{
	return e->next;
}

/**
 * Return the previous element, or NULL if there is no previous element.
 * @see lh_table_head()
 * @see lh_entry_next()
 */
static _LH_INLINE struct lh_entry *lh_entry_prev(const struct lh_entry *e)
{
	return e->prev;
}

#undef _LH_INLINE

#ifdef __cplusplus
}
#endif

#endif
