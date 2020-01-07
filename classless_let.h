#pragma once

#if defined(__clang__)
# define CLS_LET(V, X) __auto_type V = (X)
#elif defined(__GNUC__) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)
# define CLS_LET(V, X) __auto_type V = (X)
#else
# define CLS_LET(V, X) __typeof(X) V = (X)
#endif

#define CLS_HEADER_OF(TYPE, P)                          \
        ((struct TYPE *)((uintptr_t)(P) - sizeof(struct TYPE)))

#define CLS_LET_HEADER(TYPE, V, P)                              \
        struct TYPE *V = (void *)((uintptr_t)(P) - sizeof(*V))

#define CLS_LET_STRIPPED(N, V, P)                               \
        CLS_TAG_STRIP(N, (P)) V = (void *)(uintptr_t)(P)

#define CLS_LET_STRIPPED_ALL(N, V, P)                           \
        CLS_TAG_STRIP_ALL(N, (P)) V = (void *)(uintptr_t)(P)

#define CLS_NONNULL_IF(P, C)                            \
        ({                                              \
                CLS_LET(cls_nonnull_ptr_, (P));         \
                                                        \
                if ((C) && cls_nonnull_ptr_ == NULL)    \
                        __builtin_unreachable();        \
                cls_nonnull_ptr_;                       \
        })


#define CLS_NONNULL(P) CLS_NONNULL_IF((P), 1)
