#pragma once
/*
 * A buf can be either a read-only view, a read-write view, a
 * write-only buffer, or a read-write view and write-only buffer.
 *
 * As a read-only view, a buf is a pointer to const data and a size.
 *
 * As a read-write view, it's a pointer to mutable data and a size.
 *
 * As a write-only buffer, a buf is a pointer to mutable scratch
 * space, a size / written index, and a capacity.  Whenever the
 * written index is updated, a reference to the real size of the
 * backing buffer is also updated.  This denormalisation helps lower
 * the indirection and makes it possible to have a writable buffer
 * that points into an arbitrary index of a parent container.
 *
 * Finally, as a read-write view and write-only buffer combined, a buf
 * is the same as a write-only buffer, except with prepopulated data
 * and a non-zero initial size.
 *
 * In all cases, the buf is represented as an array of pointers to
 * data, of which two are actually integers, and one a pointer to
 * `size_t`; the buf is passed around as a pointer to the last
 * dummy item in that array, a NULL pointer.
 *
 * These bufs are meant to be the main form of generic iteration in
 * classless, especially for writes: write-only buffers provide a
 * convenient interface for bulk insertion.
 */

#include <assert.h>
#include <stddef.h>

#include "classless.h"


/*
 * The pointer to array of pointers is tagged with address space 100.
 */
CLS_TAG_REGISTER(classless_buf, 100);

enum {
        CLS_BUF_IDX_SIZE_PTR = -4,
        CLS_BUF_IDX_CAPACITY = -3,
        CLS_BUF_IDX_SIZE = -2,
        CLS_BUF_IDX_DATA = -1,
};

/*
 * A read-only view is `const T cls_buf const *`.
 * A read-write view is `T cls_buf const *`.
 * A read-write buffer is `T cls_buf *`.
 */
#define cls_buf * CLS_TAG(classless_buf)

/*
 * Creates backing storage for a buf: it's an array of pointers to the
 * buf's data type.
 *
 * DATA is the data buffer
 * SIZE_PTR is NULL for views, or a pointer to `DATA`'s actual
 *     size for writable buffers
 * SIZE is the number of items already written to DATA
 * CAPACITY is the total number of items available to read or write in DATA
 */
#define cls_buf_block(DATA, SIZE_PTR, SIZE, CAPACITY)    \
        ((__typeof__(__typeof(*(DATA)) *)[]) {           \
                (void *)(uintptr_t)(SIZE_PTR),           \
                (void *)(uintptr_t)(CAPACITY),           \
                (void *)(uintptr_t)(SIZE),               \
                &(DATA)[0],                              \
                (void *)0,                               \
        })

/*
 * Converts a block to a mutable buf argument.  This trick relies on
 * lifetime extension for function arguments, and must only be used
 * directly as a function (not macro!)'s argument.
 */
#define cls_buf_ref(BLOCK)                                              \
        ((__typeof__((BLOCK)[4][0]) * CLS_TAG(classless_buf) *)         \
         (uintptr_t)(&(BLOCK)[4]))

/*
 * Converts a block to a view argument.
 */
#define cls_buf_view(BLOCK)                                             \
        ((__typeof__((BLOCK)[4][0]) * const CLS_TAG(classless_buf) *)   \
         (uintptr_t)(&(BLOCK)[4]))

/*
 * Converts a block to a const view argument.
 */
#define cls_buf_const_view(BLOCK)                                       \
        ((const __typeof__((BLOCK)[4][0]) *const CLS_TAG(classless_buf) *) \
         (uintptr_t)(&(BLOCK)[4]))

/*
 * Returns a pointer to the buf or view's data.
 */
#define cls_buf_data(BUF)                                       \
        ({                                                      \
                CLS_LET(cls_buf_, (BUF));                       \
                CLS_TAG_CHECK(classless_buf, cls_buf_);         \
                                                                \
                cls_buf_[CLS_BUF_IDX_DATA];                     \
        })

/*
 * Returns the buf or view's size.
 */
#define cls_buf_size(BUF)                                       \
        ({                                                      \
                CLS_LET(cls_buf_, (BUF));                       \
                CLS_TAG_CHECK(classless_buf, cls_buf_);         \
                                                                \
                (size_t)(uintptr_t)cls_buf_[CLS_BUF_IDX_SIZE];  \
        })

/*
 * Updates the buf's size.
 *
 * Returns true if the update was successful (the new size does not
 * exceed the capacity); does nothing and returns false otherwise.
 */
#define cls_buf_set_size(BUF, SIZE)                                     \
        ({                                                              \
                CLS_LET(cls_buf_, (BUF));                               \
                CLS_MUTABLE_TAG_CHECK(classless_buf, cls_buf_);         \
                size_t cls_buf_size_ = (SIZE);                          \
                size_t cls_buf_capacity_ = (uintptr_t)cls_buf_[CLS_BUF_IDX_CAPACITY]; \
                size_t cls_buf_curr_size_ = (uintptr_t)cls_buf_[CLS_BUF_IDX_SIZE]; \
                size_t *cls_buf_size_ptr_ =                             \
                        (void *)(uintptr_t)cls_buf_[CLS_BUF_IDX_SIZE_PTR]; \
                                                                        \
                (cls_buf_size_ <= cls_buf_capacity_)                    \
                        && (cls_buf_size_ptr_ +=                        \
                            (cls_buf_size_ - cls_buf_curr_size_),       \
                            cls_buf_[CLS_BUF_IDX_SIZE] =                \
                            (void *)(uintptr_t)cls_buf_size_,           \
                            true);                                      \
        })

/*
 * Returns the buf's total capacity.
 */
#define cls_buf_capacity(BUF)                                           \
        ({                                                              \
                CLS_LET(cls_buf_, (BUF));                               \
                CLS_MUTABLE_TAG_CHECK(classless_buf, cls_buf_);         \
                                                                        \
                (size_t)(uintptr_t)cls_buf_[CLS_BUF_IDX_CAPACITY];      \
        })

/*
 * Returns a reference to the i'th value in the view or buf.
 *
 * Performs bound checking when asserts are enabled.
 */
#define cls_buf_at(BUF, I) (*CLS_BUF_AT_((BUF), (I)))

#define CLS_BUF_AT_(BUF, I)                                             \
        ({                                                              \
                CLS_LET(cls_buf_, (BUF));                               \
                CLS_TAG_CHECK(classless_buf, cls_buf_);                 \
                size_t cls_buf_i_ = (I);                                \
                size_t cls_buf_size_ = (uintptr_t)cls_buf_[CLS_BUF_IDX_SIZE]; \
                                                                        \
                assert(cls_buf_i_ < cls_buf_size_);                     \
                (cls_buf_i_ < cls_buf_size_)                            \
                        ? CLS_NONNULL(&cls_buf_[CLS_BUF_IDX_DATA][cls_buf_i_]) \
                        :  NULL;                                        \
        })

/*
 * Attempts to add `X` at the end of the `BUF`.
 *
 * Returns true on success, false if the buf is at capacity.
 */
#define cls_buf_push(BUF, X)                                            \
        ({                                                              \
                CLS_LET(cls_buf_, (BUF));                               \
                CLS_MUTABLE_TAG_CHECK(classless_buf, cls_buf_);         \
                __typeof__(**cls_buf_) cls_buf_x_ = (X);                \
                size_t cls_buf_size_ = (uintptr_t)cls_buf_[CLS_BUF_IDX_SIZE]; \
                size_t cls_buf_cap_ = (uintptr_t)cls_buf_[CLS_BUF_IDX_CAPACITY]; \
                size_t *cls_buf_size_ptr_ =                             \
                        (void *)(uintptr_t)cls_buf_[CLS_BUF_IDX_SIZE_PTR]; \
                                                                        \
                (cls_buf_size_ < cls_buf_cap_)                          \
                        && (memcpy(&cls_buf_[CLS_BUF_IDX_SIZE][cls_buf_size_++], \
                                   &cls_buf_x_,                         \
                                   sizeof(cls_buf_x_)),                 \
                            (*cls_buf_size_ptr_)++,                     \
                            (cls_buf_[CLS_BUF_IDX_SIZE] =               \
                             (void *)(uintptr_t)cls_buf_size_),         \
                            true);                                      \
        })

/*
 * Attempts to add one more element to the mutable buf.
 *
 * Returns a pointer to the new element on success, false on failure.
 */
#define cls_buf_bump(BUF)                                               \
        ({                                                              \
                CLS_LET(cls_buf_, (BUF));                               \
                CLS_MUTABLE_TAG_CHECK(classless_buf, cls_buf_);         \
                size_t cls_buf_size_ = (uintptr_t)cls_buf_[CLS_BUF_IDX_SIZE]; \
                size_t cls_buf_cap_ = (uintptr_t)cls_buf_[CLS_BUF_IDX_CAPACITY]; \
                size_t *cls_buf_size_ptr_ =                             \
                        (void *)(uintptr_t)cls_buf_[CLS_BUF_IDX_SIZE_PTR]; \
                                                                        \
                (cls_buf_size_ < cls_buf_cap_)                          \
                	? ((*cls_buf_size_ptr_)++,                      \
                           (cls_buf_[CLS_BUF_IDX_SIZE] = (void *)(cls_buf_size_ + 1)), \
                           CLS_NONNULL(&cls_buf_[CLS_BUF_IDX_DATA][cls_buf_size_])) \
                	: NULL;                                         \
        })

/*
 * Attempts to reserve N elements in the mutable buf.
 *
 * Returns a pointer to the tail of the buf if there's enough capacity
 * for N new elements, NULL otherwise.
 */
#define cls_buf_reserve(BUF, N)                                         \
        ({                                                              \
                CLS_LET(cls_buf_, (BUF));                               \
                CLS_MUTABLE_TAG_CHECK(classless_buf, cls_buf_);         \
                size_t cls_buf_n_ = (N);                                \
                size_t cls_buf_size_ = (uintptr_t)cls_buf_[CLS_BUF_IDX_SIZE]; \
                size_t cls_buf_cap_ = (uintptr_t)cls_buf_[CLS_BUF_IDX_CAPACITY]; \
                                                                        \
                (cls_buf_n_ <= cls_buf_cap_ - cls_buf_size_)            \
                        ? CLS_NONNULL_IF(&cls_buf_[CLS_BUF_IDX_DATA][cls_buf_size_],\
                                         cls_buf_n_ > 0)                \
                        : NULL;                                         \
        })

/*
 * Commits N previously reserved elements in the mutable buf.
 *
 * Increments the buf's size by n and returns true if there is enough
 * capacity, does nothing and returns false otherwise.
 */
#define cls_buf_commit(BUF, N)                                          \
        ({                                                              \
                CLS_LET(cls_buf_, (BUF));                               \
                CLS_MUTABLE_TAG_CHECK(classless_buf, cls_buf_);         \
                size_t cls_buf_n_ = (N);                                \
                size_t cls_buf_size_ = (uintptr_t)cls_buf_[CLS_BUF_IDX_SIZE]; \
                size_t cls_buf_cap_ = (uintptr_t)cls_buf_[CLS_BUF_IDX_CAPACITY]; \
                size_t *cls_buf_size_ptr_ =                             \
                        (void *)(uintptr_t)cls_buf_[CLS_BUF_IDX_SIZE_PTR]; \
                                                                        \
                (cls_buf_n_ <= cls_buf_cap_ - cls_buf_size_)            \
                        && ((*cls_buf_size_ptr_) += cls_buf_n_,         \
                            cls_buf_[CLS_BUF_IDX_SIZE] =                \
                            (void *)(cls_buf_size_ + cls_buf_n_),       \
                            true);                                      \
        })
