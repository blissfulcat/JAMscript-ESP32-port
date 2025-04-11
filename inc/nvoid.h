/** @addtogroup nvoid
 * @{
 * @brief The nvoid module is simply a definition of a custom type, which stores a void pointer (a pointer to
 * any possible structure) and a length n (hence the name nvoid). It is one of the types which can be
 * passed through the JAMScript commands.
 */

/*
The MIT License (MIT)
Copyright (c) 2016 Muthucumaru Maheswaran

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __NVOID_H__
#define __NVOID_H__
#include "utils.h"

/**
 * @brief Struct defining the nvoid type.
*/
typedef struct _nvoid_t
{
    int len;    ///< length of the nvoid object
    void *data; ///< pointer to the data
} nvoid_t;

/**
 * @brief Constructor. Allocates memory dynamically to create a new nvoid object.
 * @param data void pointer to the data
 * @param len length of that data
 * @return pointer to the newly created nvoid object.
*/
nvoid_t *nvoid_new(void *data, int len);

/**
 * @brief Creates a new nvoid object which by default points to null. (&data=0, len=0).
 * @return pointer to the newly created nvoid object.
*/
nvoid_t *nvoid_null();

/**
 * @brief Free the memory allocated to the nvoid object.
*/
#define nvoid_free(n)  do {             \
    free(n);                            \
} while (0)
#endif
/**
 * @}
*/
