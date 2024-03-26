// AndiRPG -- Name not final
// Copyright © 2024 Massimo Gengarelli
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __COLLECTIONS_LINKED_LIST__H__
#define __COLLECTIONS_LINKED_LIST__H__

// Implementation of a null-terminated linked list in C
#include <stdint.h>
typedef struct LinkedList LinkedList;

typedef void (*FreeFunction)(void *);
typedef bool (*Comparator)(void const *);

// Constructors and deconstructors
LinkedList *linked_list_new(uint32_t, FreeFunction);
void        linked_list_free(LinkedList *);

bool     linked_list_is_empty(LinkedList const *);
uint32_t linked_list_get_current_size(LinkedList const *);

// Methods
void     linked_list_add(LinkedList *, void *);
void     linked_list_remove(LinkedList *, uint32_t);
uint32_t linked_list_count(LinkedList const *);

// Iterator pattern
bool  linked_list_iterator_has_next(LinkedList const *);
void *linked_list_iterator_next(LinkedList *);
void  linked_list_iterator_reset(LinkedList *);

// Memory management
void linked_list_memory_extend(LinkedList *self, uint32_t size);
void linked_list_memory_shrink(LinkedList *self);

// Get the element at the given index
void *linked_list_get(LinkedList const *, uint32_t);

// Find the first element for which the passed function returns true
void *linked_list_find(LinkedList const *, Comparator);

// Same as before but returns a null-terminated list of *all* the elements matching
void **linked_list_find_all(LinkedList const *, Comparator, size_t *size);

#endif /* ifndef __COLLECTIONS_LINKED_LIST__H__ */
