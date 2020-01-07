#pragma once

/*
 * We use dummy address spaces (and additional `sparse` attributes) to
 * detect incorrect usage, like type confusion.
 *
 * Unfortunately, we need extensions in order to do that.  We only
 * know how to use clang's and `sparse`'s implementations.
 */

#if defined(__CHECKER__)
# include "classless_tag_sparse.h"
#elif defined(__clang__)
# include "classless_tag_clang.h"
#else
# include "classless_tag_fallback.h"
#endif

#define CLS_TAG_REGISTER(NAME, TAG)                     \
        enum {                                          \
                classless_tag_number_##TAG = (TAG),     \
                classless_tag_##NAME = (TAG),           \
        }
