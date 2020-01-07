#pragma once

/*
 * We tag pointers that are really different data structures with
 * dummy address spaces.  These do not affect code generation, but
 * are propagated in the type system.
 */
#define CLS_TAG(N) __attribute__((__address_space__(classless_tag_##N)))

#define CLS_IS_TAG(N, P)                                                \
        (__builtin_types_compatible_p(const void CLS_TAG(N) *,          \
                                      __typeof__(0 ? (P) : (const void *)1)))

#define CLS_IS_MUTABLE_TAG(N, P)                                        \
        (__builtin_types_compatible_p(void CLS_TAG(N) *,                \
                                      __typeof__(0 ? (P) : (void *)1)))

#define CLS_TAG_CHECK(N, P, ...)                                        \
        _Static_assert(CLS_IS_TAG(N, P),                                \
                       "Expected tagged (" #N ") non-volatile pointer. " \
                       __VA_ARGS__ )

#define CLS_MUTABLE_TAG_CHECK(N, P, ...)                                \
        _Static_assert(CLS_IS_MUTABLE_TAG(N, P),                        \
                       "Expected mutable tagged (" #N ") non-volatile pointer. " \
                       __VA_ARGS__ )
/*
 * Strips all cv and address space from the pointer type of P.
 */
#define CLS_TAG_STRIP_ALL(N, P)                                         \
        __typeof__(__builtin_choose_expr(CLS_IS_TAG(N, P),		\
                                         (CLS_TAG_POINTEE(P) *)0,       \
                                         (void)0))

#define CLS_TAG_STRIPPED_ALL(N, P) ((CLS_TAG_STRIP_ALL(N, P))(uintptr_t)(P))

#define CLS_TAG_CONST_STRIP(N, P)                                       \
        __typeof__(__builtin_choose_expr(CLS_IS_TAG(N, P),		\
                                         (CLS_TAG_POINTEE(P) const *)0, \
                                         (void)0))

#define CLS_TAG_CONST_STRIPPED(N, P) ((CLS_TAG_CONST_STRIP(N, P))(uintptr_t)(P))

/*
 * Strips volatile and address space from the pointer type of P.
 */
#define CLS_TAG_STRIP(N, P)                                             \
        __typeof__(__builtin_choose_expr(CLS_IS_TAG(N, P),              \
                                         CLS_TAG_STRIP_(N, P),          \
                                         (void)0))

#define CLS_TAG_STRIPPED(N, P) ((CLS_TAG_STRIP(N, P))(uintptr_t)(P))

#define CLS_TAG_STRIP_(N, P)                                            \
        __builtin_choose_expr(CLS_TAG_IS_CONST(N, P),                   \
                              (const CLS_TAG_POINTEE(P) *)0,            \
                              (CLS_TAG_POINTEE(P) *)0)

/*
 * The statement expression evalutes to an rvalue, so we lose all
 * qualifications on the value.  This gives us the type of the
 * pointee, without any of the noise.
 */
#define CLS_TAG_POINTEE(P) __typeof__(({* (P); }))

#define CLS_TAG_IS_CONST(N, P)                                          \
        __builtin_types_compatible_p(const volatile void CLS_TAG(N) *,  \
                                     __typeof__(0 ? P : (volatile void *)1))
