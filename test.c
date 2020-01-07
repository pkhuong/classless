#include "classless.h"

#include "classless_buf.h"
#include "classless_vec.h"

cls_vec int *
create(size_t n)
{

        return cls_vec_create(int, n);
}

void
destroy(cls_vec int *v)
{

        cls_vec_destroy(v);
        return;
}

int
foo(cls_vec const int *vec)
{
        return cls_vec_size(vec) == 0 ? 0 : vec[1];
}

int
bar(cls_vec const int *vec)
{

        return cls_vec_at(vec, 2);
}

int *
baz(cls_vec int *vec)
{

        return cls_vec_data(vec);
}

const int *
const_baz(cls_vec const int *vec)
{

        return cls_vec_data(vec);
}

void
test_push(cls_vec int *vec)
{

        cls_vec_push(vec, 1);
        cls_vec_push(vec, 2);
        return;
}

void
test_bump(cls_vec int *vec)
{

        *cls_vec_bump(vec) = 1;
        *cls_vec_bump(vec) = 2;
        return;
}

void
test_reserve(cls_vec int *restrict vec)
{
        int *restrict dst;

        dst = cls_vec_reserve(vec, 2);
        if (dst != NULL) {
                dst[0] = 1;
                dst[2] = 2;
                cls_vec_commit(vec, 2);
        }

        return;
}

size_t
test_capacity(const cls_vec int *vec)
{

        return cls_vec_capacity(vec);
}

bool
test_set_size(cls_vec int *vec)
{

        return cls_vec_set_size(vec, 42);
}

int
buf_bar(const int cls_buf *buf)
{

        return cls_buf_at(buf, 2);
}

int *
buf_baz(int cls_buf *buf)
{

        return cls_buf_data(buf);
}

const int *
buf_const_baz(const int cls_buf const *buf)
{

        return cls_buf_data(buf);
}

void
buf_test_push(int cls_buf *buf)
{

        cls_buf_push(buf, 1);
        cls_buf_push(buf, 2);
        return;
}

void
buf_test_bump(int cls_buf *buf)
{

        *cls_buf_bump(buf) = 1;
        *cls_buf_bump(buf) = 2;
        return;
}

void
buf_test_reserve(int cls_buf *buf)
{
        int *restrict dst;

        dst = cls_buf_reserve(buf, 2);
        if (dst != NULL) {
                dst[0] = 1;
                dst[2] = 2;
                cls_buf_commit(buf, 2);
        }

        return;
}

size_t
buf_test_capacity(int cls_buf *buf)
{

        return cls_buf_capacity(buf);
}

bool
buf_test_set_size(const int cls_buf *buf)
{

        return cls_buf_set_size(buf, 42);
}

static int
test_borrowed_buf_sum(const int cls_buf const* buf)
{

        return cls_buf_at(buf, 0) + cls_buf_at(buf, 1);
}

int
test_vec_buf_sum(cls_vec int *vec)
{

        return test_borrowed_buf_sum(cls_vec_const_view(vec, 0));
}

void
test_vec_buf_reserve(cls_vec int *vec)
{

        buf_test_reserve(cls_vec_buf(vec, 0, -1));
        return;
}

void
test_vec_buf_push(cls_vec int *vec)
{

        buf_test_push(cls_vec_buf_tail(vec, -1));
        return;
}
