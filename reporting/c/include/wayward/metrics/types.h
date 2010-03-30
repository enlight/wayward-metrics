#ifndef _WAYWARD_METRICS_TYPES_H_
#define _WAYWARD_METRICS_TYPES_H_

#ifdef WIN32

typedef signed __int8  int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;

typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

#else // if not WIN32
#include <stdint.h>
#endif // WIN32

#endif // _WAYWARD_METRICS_TYPES_H_

