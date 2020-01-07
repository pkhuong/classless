#pragma once
/*
 * A vec is a fixed-capacity vector.  It differs from a buf by shaving
 * another level of indirection, and by allowing direct indexing of
 * its data: the vec is a pointer to the data vector.
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "classless.h"
#include "classless_buf.h"

struct classless_vec_header {
        size_t capacity;
        size_t size;
};

/*
 * The pointer to the data array is tagged with address space 101.
 */
CLS_TAG_REGISTER(classless_vec, 101);

/*
 * A vec is `T cls_vec *`.
 * A const vec (to const) is `const T cls_vec *`.
 */
#define cls_vec CLS_TAG(classless_vec)

/*
 * Allocates a vec of T with the specified capacity, or returns NULL
 * on allocation failure.
 *
 * The data is not zero-filled.
 */
#define cls_vec_create(T, CAPACITY)                                     \
        ((__typeof__(T) cls_vec *) classless_vec_create_((CAPACITY), sizeof(T)))

/*
 * Deallocates a vec.  Safe to call on NULL.
 */
#define cls_vec_destroy(VEC)                                            \
        ({                                                              \
                CLS_LET(cls_vec_, (VEC));                               \
                CLS_MUTABLE_TAG_CHECK(classless_vec, cls_vec_);         \
                                                                        \
                classless_vec_destroy_(CLS_TAG_STRIPPED(classless_vec, cls_vec_)); \
        })

/*
 * Converts a vec to a view of its data starting at OFFSET.
 */
#define cls_vec_view(VEC, OFFSET) cls_buf_view(CLS_VEC_VIEW_((VEC), (OFFSET)))

/*
 * Converts a vec to a const view of its data starting at OFFSET.
 */
#define cls_vec_const_view(VEC, OFFSET)                         \
        cls_buf_const_view(CLS_VEC_VIEW_((VEC), (OFFSET)))

#define CLS_VEC_VIEW_(VEC, OFFSET)                                      \
        ({                                                              \
                CLS_LET(cls_vec_, VEC);                                 \
                size_t cls_vec_offset_ = OFFSET;                        \
                CLS_TAG_CHECK(classless_vec, cls_vec_);                 \
                CLS_LET_HEADER(classless_vec_header, cls_vec_h_, cls_vec_); \
                                                                        \
                if (cls_vec_offset_ > cls_vec_h_->size)                 \
                        cls_vec_offset_ = cls_vec_h_->size;             \
                                                                        \
                cls_buf_block(                                          \
                        &CLS_TAG_STRIPPED(classless_vec, cls_vec_)[cls_vec_offset_], \
                        NULL,                                           \
                        cls_vec_h_->size - cls_vec_offset_,             \
                        cls_vec_h_->capacity - cls_vec_offset_);        \
        })

/*
 * Converts a mutable vec to a buf of its data starting at OFFSET, and with
 * room for up to TAIL additional elements.
 */
#define cls_vec_buf(VEC, OFFSET, TAIL)                          \
        cls_buf_ref(CLS_VEC_BUF_((VEC), (OFFSET), (TAIL)))

/*
 * Converts a mutable vec to a buf of its unpopulated data, with room
 * for up to TAIL new elements.
 */
#define cls_vec_buf_tail(VEC, TAIL)                             \
        cls_buf_ref(CLS_VEC_BUF_TAIL_((VEC), (TAIL)))

#define CLS_VEC_BUF_(VEC, OFFSET, TAIL)                                 \
        ({                                                              \
                CLS_LET(cls_vec_, VEC);                                 \
                size_t cls_vec_offset_ = OFFSET;                        \
                size_t cls_vec_tail_ = TAIL;                            \
                size_t cls_vec_remaining_;                              \
                CLS_MUTABLE_TAG_CHECK(classless_vec, cls_vec_);         \
                CLS_LET_HEADER(classless_vec_header, cls_vec_h_, cls_vec_); \
                                                                        \
                if (cls_vec_offset_ > cls_vec_h_->size)                   \
                        cls_vec_offset_ = cls_vec_h_->size;             \
                if (cls_vec_tail_ >= cls_vec_h_->capacity - cls_vec_h_->size) \
                        cls_vec_tail_ = cls_vec_h_->capacity - cls_vec_h_->size; \
                                                                        \
                cls_vec_remaining_ = cls_vec_h_->size - cls_vec_offset_; \
                cls_buf_block(                                          \
                        &CLS_TAG_STRIPPED(classless_vec, cls_vec_)[     \
                                cls_vec_offset_],                       \
                        &cls_vec_h_->size,                              \
                        cls_vec_remaining_,                             \
                        cls_vec_remaining_ + cls_vec_tail_);            \
        })

#define CLS_VEC_BUF_TAIL_(VEC, TAIL)                                    \
        ({                                                              \
                CLS_LET(cls_vec_, VEC);                                 \
                size_t cls_vec_tail_ = TAIL;                            \
                CLS_MUTABLE_TAG_CHECK(classless_vec, cls_vec_);         \
                CLS_LET_HEADER(classless_vec_header, cls_vec_h_, cls_vec_); \
                                                                        \
                if (cls_vec_tail_ >= cls_vec_h_->capacity - cls_vec_h_->size) \
                        cls_vec_tail_ = cls_vec_h_->capacity - cls_vec_h_->size; \
                                                                        \
                cls_buf_block(                                          \
                        &CLS_TAG_STRIPPED(classless_vec, cls_vec_)[     \
                                cls_vec_h_->size],                      \
                        &cls_vec_h_->size,                              \
                        0,                                              \
                        cls_vec_tail_);                                 \
        })

/*
 * Returns a reference to the i'th value in the vec.
 *
 * Performs bound checking when asserts are enabled.
 */
#define cls_vec_at(VEC, I) (*CLS_VEC_AT_((VEC), (I)))

#define CLS_VEC_AT_(VEC, I)                                             \
        ({                                                              \
                CLS_LET(cls_vec_, VEC);                                 \
                size_t cls_vec_i_ = I;                                 \
                CLS_TAG_CHECK(classless_vec, cls_vec_);                 \
                CLS_LET_HEADER(classless_vec_header, cls_vec_h_, cls_vec_); \
                                                                        \
                assert(cls_vec_i_ < cls_vec_h_->size);                  \
                (cls_vec_i_ < cls_vec_h_->size)                         \
                        ? &CLS_TAG_STRIPPED(classless_vec, cls_vec_)[cls_vec_i_] \
                        : NULL;                                         \
        })                                                              \

/*
 * Returns a pointer to the vec's data.
 */
#define cls_vec_data(VEC)                                               \
        ({                                                              \
                CLS_LET(cls_vec_, (VEC));                               \
                CLS_TAG_CHECK(classless_vec, cls_vec_);                 \
                                                                        \
                CLS_TAG_STRIPPED(classless_vec, cls_vec_);              \
        })

/*
 * Attempts to add `X` at the end of `VEC.
 *
 * Returns true on success, false if the vec is at capacity.
 */
#define cls_vec_push(VEC, X)                                            \
        ({                                                              \
                CLS_LET(cls_vec_, (VEC));                               \
                __typeof__(*CLS_TAG_STRIPPED(classless_vec, cls_vec_))  \
                        cls_vec_x_ = (X);                               \
                CLS_MUTABLE_TAG_CHECK(classless_vec, cls_vec_);         \
                CLS_LET_STRIPPED(classless_vec, cls_vec_ptr_, cls_vec_); \
                CLS_LET_HEADER(classless_vec_header, cls_vec_h_, cls_vec_); \
                                                                        \
                (cls_vec_h_->size < cls_vec_h_->capacity)               \
                        && (memcpy(&cls_vec_ptr_[cls_vec_h_->size++],   \
                                   &cls_vec_x_,                         \
                                   sizeof(cls_vec_x_)), true);          \
        })

/*
 * Attempts to add one more element to the mutable vec.
 *
 * Returns a pointer to the new element on success, false on failure.
 */
#define cls_vec_bump(VEC)                                               \
        ({                                                              \
                CLS_LET(cls_vec_, (VEC));                               \
                CLS_MUTABLE_TAG_CHECK(classless_vec, cls_vec_);         \
                CLS_LET_STRIPPED(classless_vec, cls_vec_ptr_, cls_vec_); \
                CLS_LET_HEADER(classless_vec_header, cls_vec_h_, cls_vec_); \
                                                                        \
                (cls_vec_h_->size < cls_vec_h_->capacity)               \
                        ? &cls_vec_ptr_[cls_vec_h_->size++]             \
                        : NULL;                                         \
        })

/*
 * Attempts to reserve N elements in the mutable vec.
 *
 * Returns a pointer to the tail of the vec if there's enough capacity
 * for N new elements, NULL otherwise.
 */
#define cls_vec_reserve(VEC, N)                                         \
        ({                                                              \
                CLS_LET(cls_vec_, (VEC));                               \
                size_t cls_vec_n_ = (N);                                \
                CLS_MUTABLE_TAG_CHECK(classless_vec, cls_vec_);         \
                CLS_LET_STRIPPED(classless_vec, cls_vec_ptr_, cls_vec_); \
                CLS_LET_HEADER(classless_vec_header, cls_vec_h_, cls_vec_); \
                                                                        \
                (cls_vec_n_ <= cls_vec_h_->capacity - cls_vec_h_->size) \
                        ? &cls_vec_ptr_[cls_vec_h_->size]               \
                        : NULL;                                         \
        })  

/*
 * Commits N previously reserved elements in the mutable vec.
 *
 * Increments the vec's size by n and returns true if there is enough
 * capacity, does nothing and returns false otherwise.
 */
#define cls_vec_commit(VEC, N)                                          \
        ({                                                              \
                CLS_LET(cls_vec_, (VEC));                               \
                size_t cls_vec_n_ = (N);                                \
                CLS_MUTABLE_TAG_CHECK(classless_vec, cls_vec_);         \
                CLS_LET_HEADER(classless_vec_header, cls_vec_h_, cls_vec_); \
                                                                        \
                (cls_vec_n_ <= cls_vec_h_->capacity)                    \
                && (cls_vec_h_->size += cls_vec_n_, true);               \
        })  

#define cls_vec_size(VEC)                                               \
        ({                                                              \
                CLS_LET(cls_vec_, (VEC));                               \
                CLS_TAG_CHECK(classless_vec, cls_vec_);                 \
                                                                        \
                (cls_vec_ == NULL)                                      \
                ? 0                                                     \
                : CLS_HEADER_OF(classless_vec_header, cls_vec_)->size;  \
        })

#define cls_vec_set_size(VEC, SIZE)                                     \
        ({                                                              \
                CLS_LET(cls_vec_, (VEC));                               \
                size_t cls_vec_size_ = (SIZE);                          \
                CLS_MUTABLE_TAG_CHECK(classless_vec, cls_vec_);         \
                CLS_LET_HEADER(classless_vec_header, cls_vec_h_, cls_vec_); \
                                                                        \
                (cls_vec_size_ <= cls_vec_h_->capacity)                 \
                && (cls_vec_h_->size = cls_vec_size_, true);            \
        })

#define cls_vec_capacity(VEC)                                           \
        ({                                                              \
                CLS_LET(cls_vec_, (VEC));                               \
                CLS_TAG_CHECK(classless_vec, cls_vec_);                 \
                                                                        \
                (cls_vec_ == NULL)                                      \
                ? 0                                                     \
                : CLS_HEADER_OF(classless_vec_header, cls_vec_)->capacity;  \
        })

static inline void *
classless_vec_create_(size_t capacity, size_t elsize)
{
        struct classless_vec_header *h;

        /* XXX: overflow. */
        h = malloc(sizeof(*h) + capacity * elsize);
        h->size = 0;
        h->capacity = capacity;
        return h + 1;
}

static inline void
classless_vec_destroy_(void *data)
{
        struct classless_vec_header *h;

        if (data == NULL)
                return;

        h = (void *)((uintptr_t)data - sizeof(*h));
        free(h);
        return;
}
