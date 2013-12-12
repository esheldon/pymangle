// This header was auto-generated
#ifndef _STACK_H
#define _STACK_H
#include <stdint.h>

#define STACK_PUSH_REALLOC_MULT 1
#define STACK_PUSH_REALLOC_MULTVAL 2
#define STACK_PUSH_INITSIZE 1

struct i64stack {
    size_t size;            // number of elements that are visible to the user
    size_t allocated_size;  // number of allocated elements in data vector
    size_t push_realloc_style; // Currently always STACK_PUSH_REALLOC_MULT, 
                               // which is reallocate to allocated_size*realloc_multval
    size_t push_initsize;      // default size on first push, default STACK_PUSH_INITSIZE 
    double realloc_multval; // when allocated size is exceeded while pushing, 
                            // reallocate to allocated_size*realloc_multval, default 
                            // STACK_PUSH_REALLOC_MULTVAL
                            // if allocated_size was zero, we allocate to push_initsize
    int64_t* data;
};

struct i64stack* i64stack_new(size_t num);

// if size > allocated size, then a reallocation occurs
// if size <= internal size, then only the ->size field is reset
// use i64stack_realloc() to reallocate the data vector and set the ->size
void i64stack_resize(struct i64stack* stack, size_t newsize);

// perform reallocation on the underlying data vector. This does
// not change the size field unless the new size is smaller
// than the viewed size
void i64stack_realloc(struct i64stack* stack, size_t newsize);

// completely clears memory in the data vector
void i64stack_clear(struct i64stack* stack);

// clears all memory and sets pointer to NULL
// usage: stack=i64stack_delete(stack);
struct i64stack* i64stack_delete(struct i64stack* stack);

// if reallocation is needed, size is increased by 50 percent
// unless size is zero, when it 100 are allocated
void i64stack_push(struct i64stack* stack, int64_t val);
// pop the last element and decrement size; no reallocation is performed
// if empty, INT64_MIN is returned
int64_t i64stack_pop(struct i64stack* stack);

int __i64stack_compare_el(const void *a, const void *b);
void i64stack_sort(struct i64stack* stack);
int64_t* i64stack_find(struct i64stack* stack, int64_t el);

#endif  // header guard
