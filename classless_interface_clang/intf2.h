#ifdef CLASSLESS_INTERFACE_INTF2
# error "Multiple (direct?) inclusion."
#endif

#define CLASSLESS_INTERFACE_INTF2

} classless_intf2;

typedef const __typeof__(classless_intf2) classless_intf2_t;
typedef const __typeof__(*classless_intf2.element) classless_intf2_element_t;

_Static_assert(CLS_INTF_FIND(classless_intf2_t, 0) == 0,
    "Interface types must only be registered once.");

_Static_assert(CLS_INTF_FIND(classless_intf2_element_t, 0) == 0,
    "Only one interface type may be registered for each element type.");

#undef CLS_INTF_TO_INDEX
#define CLS_INTF_TO_INDEX(INTF, DEFAULT)                                \
        _Generic((const __typeof__(INTF) *)0,                           \
                 classless_intf1_t *: 1,                                \
                 classless_intf2_t *: 2,                                \
                 default: (DEFAULT))

#undef CLS_INTF_FIND
#define CLS_INTF_FIND(T, DEFAULT)                                       \
        _Generic((const __typeof__(T) *)0,                              \
                 classless_intf2_t *: classless_intf2,                  \
                 classless_intf1_t *: classless_intf1,                  \
                 default:                                               \
                 _Generic((const __typeof(T) *)0,                       \
                          classless_intf1_element_t *: classless_intf1, \
                          classless_intf2_element_t *: classless_intf2, \
                          default: (DEFAULT)))

#undef CLS_INTF_FROM_INDEX
#define CLS_INTF_FROM_INDEX(I, DEFAULT)                                 \
        _Generic((const char(*)[(I)])0,                                 \
                 const char(*)[1]: classless_intf1,                     \
                 const char(*)[2]: classless_intf2,                     \
                 default: (DEFAULT))

#undef CLS_INTF_REGISTER
#define CLS_INTF_REGISTER "classless_interface_clang/intf3.h"

__attribute__((__unused__)) static const classless_intf2_t classless_intf2 = {
