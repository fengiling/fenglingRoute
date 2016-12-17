#ifndef _HASH_H_
#define _HASH_H_

#include <stdint.h>

 uint64_t mmhash64(const void *key, int len);
 uint64_t naivehash64(const void *key, int len);

#endif
