#ifdef CLASSLESS_INTERFACE_INTF1
# error "Multiple (direct?) inclusion."
#endif

#define CLASSLESS_INTERFACE_INTF1

} classless_intf1;

typedef const __typeof__(classless_intf1) classless_intf1_t;
typedef const __typeof__(*classless_intf1.element) classless_intf1_element_t;

#undef CLS_INTF_TO_INDEX
#define CLS_INTF_TO_INDEX(INTF, DEFAULT)                                \
        _Generic((const __typeof__(INTF) *)0,                           \
                 classless_intf1_t *: 1,                                \
                 default: (DEFAULT))

#undef CLS_INTF_FIND
#define CLS_INTF_FIND(T, DEFAULT)                                       \
        _Generic((const __typeof__(T) *)0,                              \
                 classless_intf1_t *: classless_intf1,                  \
                 default:                                               \
                 _Generic((const __typeof(T) *)0,                       \
                          classless_intf1_element_t *: classless_intf1, \
                          default: (DEFAULT)))

#undef CLS_INTF_FROM_INDEX
#define CLS_INTF_FROM_INDEX(I, DEFAULT)                                 \
        _Generic((const char(*)[(I)])0,                                 \
                 const char(*)[1]: classless_intf1,                     \
                 default: (DEFAULT))

#undef CLS_INTF_REGISTER
#define CLS_INTF_REGISTER "classless_interface_clang/intf2.h"

__attribute__((__unused__)) static const classless_intf1_t classless_intf1 = {
