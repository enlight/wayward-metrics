//------------------------------------------------------------------------------
/**
    Could be used as precompiled header in the future, but for now it's just a
    convenient way to include oft used platform-specific headers all in one go
    (and in the correct order).
*/

#ifdef WIN32
#include "wayward/metrics/windows/platform.h"
#else
#include "wayward/metrics/posix/platform.h"
#endif
