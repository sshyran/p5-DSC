
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "dns_message.h"
#include "md_array.h"

void md_array_grow_d1(md_array * a);
void md_array_grow_d2(md_array * a);


md_array *
md_array_create(char *type1, IDXR * idx1, HITR * itr1,
	char *type2, IDXR * idx2, HITR * itr2)
{
    int i1;
    md_array *a = calloc(1, sizeof(*a));
    a->d1.type = type1;
    a->d1.indexer = idx1;
    a->d1.iterator = itr1;
    a->d1.alloc_sz = 2;
    a->d1.max_idx = 0;
    a->d2.type = type2;
    a->d2.indexer = idx2;
    a->d2.iterator = itr2;
    a->d2.alloc_sz = 2;
    a->d2.max_idx = 0;
    a->array = calloc(a->d1.alloc_sz, sizeof(int *));
    for (i1 = 0; i1 < a->d1.alloc_sz; i1++) {
	a->array[i1] = calloc(a->d2.alloc_sz, sizeof(int));
    }
    return a;
}

int
md_array_count(md_array * a, dns_message * m)
{
    int i1 = a->d1.indexer(m);
    int i2 = a->d2.indexer(m);

    if (i1 >= a->d1.alloc_sz)
	md_array_grow_d1(a);
    if (i2 >= a->d2.alloc_sz)
	md_array_grow_d2(a);

    assert(i1 < a->d1.alloc_sz);
    assert(i2 < a->d2.alloc_sz);
    a->array[i1][i2]++;
    return a->array[i1][i2];
}


void
md_array_grow_d1(md_array * a)
{
    int new_alloc_sz;
    int **new;
    int i1;
    fprintf(stderr, "growing d1\n");
    fprintf(stderr, "current alloc_sz=%d\n", a->d1.alloc_sz);
    new_alloc_sz = a->d1.alloc_sz << 1;
    fprintf(stderr, "new alloc_sz=%d\n", new_alloc_sz);
    new = calloc(new_alloc_sz, sizeof(int *));
    fprintf(stderr, "copying %d bytes from old to new\n", a->d1.alloc_sz * sizeof(int *));
    memcpy(new, a->array, a->d1.alloc_sz * sizeof(int *));
    free(a->array);
    a->array = new;

    for (i1 = a->d1.alloc_sz; i1 < new_alloc_sz; i1++) {
	a->array[i1] = calloc(a->d2.alloc_sz, sizeof(int));
    }

    a->d1.alloc_sz = new_alloc_sz;
}

void
md_array_grow_d2(md_array * a)
{
    int new_alloc_sz;
    int *new;
    int i1;
    new_alloc_sz = a->d2.alloc_sz << 1;
    for (i1 = 0; i1 < a->d1.alloc_sz; i1++) {
	new = calloc(new_alloc_sz, sizeof(int *));
	memcpy(new, a->array[i1], a->d2.alloc_sz * sizeof(int));
	free(a->array[i1]);
	a->array[i1] = new;
    }
    a->d2.alloc_sz = new_alloc_sz;
}

int
md_array_print(md_array * a, md_array_printer * pr)
{
    char *label1;
    char *label2;
    int i1;
    int i2;
    a->d1.iterator(NULL);
    pr->start_array();
    pr->d1_type(a->d1.type);
    pr->d2_type(a->d2.type);
    pr->start_data();
    while ((i1 = a->d1.iterator(&label1)) > -1) {
	assert(i1 < a->d1.alloc_sz);
	pr->d1_begin(label1);
	a->d2.iterator(NULL);
	while ((i2 = a->d2.iterator(&label2)) > -1) {
	    assert(i2 < a->d2.alloc_sz);
	    if (0 == a->array[i1][i2])
		continue;
	    pr->print_element(label2, a->array[i1][i2]);
	}
	pr->d1_end(label1);
    }
    pr->finish_data();
    pr->finish_array();
    return 0;
}