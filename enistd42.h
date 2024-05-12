#ifndef __ENISTD42_INCL__
#define __ENISTD42_INCL__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//In case we are using my "standard" library with C++ code
#ifdef __cplusplus
extern "C" {
#endif

#define __PILE_EXPONENT__ 2 //Exponent of how much data to reallocate when growing
#define __PILE_DEFAULT_CAPACITY__ 8 //Initial capacity of the pile

#define MAX(a, b) ((a)>(b)?(a):(b)) //Calculates maximum of 2 numbers.
#define MIN(a, b) ((a)<(b)?(a):(b)) //Calculates minimum of 2 numbers.
#define ASSERT(b, msg) {if(!(b)){printf("Assertion failed: %s, l%d: %s\n", __FILE__, __LINE__, (msg));exit(1);}} //ASSERT(you_know_what_this_does, "you really don't?");

//A pile of data. The data type is defined manually.
//Allows O(1) amortized push and pop operations from front and back
typedef struct {
    unsigned long capacity; //capacity is the capacity of data (max number of elements). Should never be 0.
    unsigned int dsize; //dsize is the size of an element
    unsigned long front, back; //Data front and back indices, modulo capacity. 0 <= front < capacity, and back - front is number of elements in pile.
    void* data; //Points to the stored data. Uses a circular buffer structure.
} Pile;

//Allocate n bytes of data on the heap, while checking if allocation was successful.
void* malloc_s(unsigned long n)
{
    void* data = malloc(n);
    if(data == NULL)
    {
        printf("Allocation of %lu bytes failed\n", n);
        exit(1);
    }
    return data;
}

#define INIT_PILE(dtype) {__PILE_DEFAULT_CAPACITY__, sizeof(dtype), 0, 0, malloc_s(__PILE_DEFAULT_CAPACITY__ * sizeof(dtype))} //Initialize an empty pile with data type dtype.
#define INIT_PILE_FROM(dtype, n, data) {MAX((n), __PILE_DEFAULT_CAPACITY__), sizeof(dtype), 0, (n), memcpy(malloc_s(sizeof(dtype) * MAX((n), __PILE_DEFAULT_CAPACITY__)), (void*)(data), (n) * sizeof(dtype))} //Initialize by copying given data. Arguments are dtype (data type), n (number of elements), data (pointer to data)
#define PILE_LENGTH(p) ((unsigned long)((long)(p).back - (long)(p).front)) //Get length of the pile p

//Push back an element on the pile. The address of the element has to be given.
void increase_if_full(Pile* p)
{
    //the pile is full, make a bigger one!
    if(p->back == p->front + p->capacity)
    {
        //increase capacity
        p->capacity *= __PILE_EXPONENT__;

        //reallocate data
        p->data = realloc(p->data, p->capacity);
        if(p->data == NULL)
        {
            printf("Allocation of %lu bytes failed\n", p->capacity);
            exit(1);
        }

        //move data before the front, to between the front and the back
        //with an exponent less than 2, we would need to do extra calculations as well
        memcpy((char*)p->data + p->dsize * ((long)p->back - (long)p->front), p->data, p->dsize * p->front);
    }
}

//Shrink the pile so that the data barely fits in it. The pile will always be full after the call.
void shrink_to_fit(Pile *p)
{
    unsigned long length = PILE_LENGTH(*p);
    
    if(p->back > p->capacity) //if data is wrapped around
    {
        //XXXXX....YYY ==> XXXXXYYY....
        memmove((char*)p->data + p->dsize * (p->back - p->capacity), (char*)p->data + p->dsize * p->front, p->dsize * (p->capacity - p->front));
        p->front = p->back - p->capacity;
    }
    else //if not wrapped around
    {
        //..XXXXYY.. --> YYXXXX....
        memmove(p->data, (char*)p->data + p->dsize * length, p->dsize * (p->back - length));
    }

    p->back = p->front + length; //in both cases the pile will end up full
    p->capacity = length; //now just shrink it!
}

#define PILE_PUSH_BACK(p, dtype, x) {increase_if_full(&p); ((dtype*)(p).data)[(p).back++] = x;} //Push x of type dtype at the back of p.
#define PILE_PUSH_FRONT(p, dtype, x) {increase_if_full(&p); if(!(p).front){(p).front+=(p).capacity;(p).back+=(p).capacity;} ((dtype*)(p).data)[--(p).front] = x;} //Push x of type dtype at the front of p.

//Return the same pile as in the argument. Throw an error and stop execution if p is empty. Also handles reducing front and back when wrapping in PILE_POP_FRONT.
//This function should not be used outside the library macros.
Pile* __throw_if_empty_and_handle_pop_front_wrapping__(Pile* p, char pop_front)
{
    if(!PILE_LENGTH(*p))
    {
        printf("Tried to pop an element of an empty pile %lu %lu\n", p->back, p->front);
        exit(1);
    }
    if((unsigned long)p->front + 1 == p->capacity && pop_front)
    {
        p->front = -1; //This intentionally overflows. Note that unsigned integer overflow is defined behavior.
        p->back -= p->capacity;
    }
    return p;
}

#define PILE_POP_BACK(p, dtype) (((dtype*)(p).data)[--__throw_if_empty_and_handle_pop_front_wrapping__(&p, 0)->back%(p).capacity]) //Pop an element of type dtype from the back pf p. Prints an error message if p is empty.
#define PILE_POP_FRONT(p, dtype) (((dtype*)(p).data)[((__throw_if_empty_and_handle_pop_front_wrapping__(&p, 1)->front++)+(p).capacity)%(p).capacity]) //Pop an element of type dtype from the front pf p. Prints an error message if p is empty.

#ifdef __cplusplus
}
#endif

#endif
