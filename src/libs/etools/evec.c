#include <string.h>
#include <assert.h>

#include "ealloc.h"
#include "etype.h"
#include "eutils.h"
#include "estr.h"
#include "estr_p.h"
#include "eobj_p.h"

#include "evec.h"


#define _EVEC_CHECK 1


#pragma pack(1)

typedef struct _evec_split_s * _evec_split, * _split;
typedef struct _evec_split_s{

    _split      prev;
    _split      next;

    //!
    //!              |-------------- front: 3
    //!              |           |-- rear : 7
    //!              |           |
    //!     |  |  |  |**|**|**|**|  |  |
    //!     0  1  2  3  4  5  6  7  8  9    cap : 10
    //!

    u16         size;       // elem size
    u16         cap;        // current capacity of this split
    int         front;      // pos of first elem
    int         rear;       // pos after the last elem

    char*       base;

}_evev_split_t;

typedef struct _pos_s
{
    //! input
    evec        v;      // evec handle
    constr      in;
    int         inlen;
    int         cnt;

    //! target
    _split      s;      // split
    int         pos;    // postion (direct)
}_pos_t, * _pos;

#pragma pack()

#define _split_prev(s)          s->prev
#define _split_next(s)          s->next
#define _split_size(s)          s->size
#define _split_fpos(s)          s->front
#define _split_rpos(s)          s->rear
#define _split_cap(s)           s->cap
#define _split_base(s)          s->base

#define _split_len(s)           (s->rear - s->front)
#define _split_avail(s)         (_split_cap(s) - (_split_len(s)))

#define _split_fptr(s)          (_split_base(s) + (_split_fpos(s) * _split_size(s)))                        // ptr of front (first element)
#define _split_rptr(s)          (_split_base(s) + (_split_rpos(s) * _split_size(s)))                        // ptr of rear
#define _split_lptr(s)          (_split_base(s) + (((_split_rear(s) - 1) % _split_cap(s)) * _split_size(s)))// ptr of last element
#define _split_eptr(s)          (_split_base(s) + (_split_cap(s) * _split_size(s)))                         // ptr of base end

#define _split_pptr(s, p)       (_split_base(s) + (p % _split_cap(s)) * _split_size(s))

#pragma pack(1)

typedef struct evec_s
{
    //! data
    u32         cnt;                // all element count
    u32         cap;                // all buffer  cap

    _split      head;               // point to the first split
    _split      tail;               // point to the last  split
    u32         splits;             // splits count
    u32         factor;             // factor for create, expand, shrink, merging, delete splits operations
    u16         esize;              // element size

    //! setting
    u16         max_cap;            // the max cap of a split can hold
    u32         reserves;           // how many items cap wanted to reserve

    u8          type;               // etypev
    uint        need_merging : 1;
}evec_t;

#pragma pack()

#define _v_head(v)          (v)->head
#define _v_tail(v)          (v)->tail
#define _v_cnt(v)           (v)->cnt
#define _v_cap(v)           (v)->cap
#define _v_esize(v)         (v)->esize
#define _v_splits(v)        (v)->splits
#define _v_factor(v)        (v)->factor
#define _v_reserve(v)       (v)->reserves
#define _v_type(v)          (v)->type

#define EVEC_MAX_PREALLOC   (1024 * 1024)       // bytes
#define EVEC_MAX_FACTOR     16


//!  factor setting
#define _merge_factor(f)    (f * 2)
#define _split_factor(f)    0
#define _incre_factor(f)    (f > 8 ? 8 * 8 * 8 : f * f * f)

#define _expand_ceiling(f)  (f * f * f)

static _split __split_new(int need_rooms, int size)
{
    _split out = ecalloc(1, sizeof(*out));

    while(need_rooms * size < 8)
    {
        need_rooms++;
    }

    out->cap   = need_rooms;
    out->size  = size;
    out->base  = ecalloc(need_rooms, size);

    return out;
}

static bool __split_expand(_split s, int need_rooms, int factor)
{
    int new_space; cstr new_base;

    new_space =  _split_size(s) * (need_rooms + _split_cap(s));

    if(new_space < EVEC_MAX_PREALLOC) new_space  = pow2gt(new_space);
    else                              new_space += EVEC_MAX_PREALLOC;

    if(new_space < _expand_ceiling(factor))
        return false;

    new_base = erealloc(s->base, new_space);

    if(!new_base)
    {
        return false;
    }

    s->base = new_base;
    s->cap  = new_space / _split_size(s);

    return true;
}

static void __split_shrink(_split s)
{

}

static void __split_fpos_shift_l(_split s, int cnt)
{
    _split_fpos(s) -= cnt;
    if(_split_fpos(s) < 0)
    {
        _split_fpos(s) += _split_cap(s);
        _split_rpos(s) += _split_cap(s);
    }
}

static void __split_fpos_shift_r(_split s, int cnt)
{
    _split_fpos(s) += cnt;
    if(_split_fpos(s) >= _split_cap(s))
    {
        _split_fpos(s) -= _split_cap(s);
        _split_rpos(s) -= _split_cap(s);
    }
}

static void __split_rpos_shift_l(_split s, int cnt)
{
    _split_rpos(s) -= cnt;
}

static void __split_rpos_shift_r(_split s, int cnt)
{
    _split_rpos(s) += cnt;
}


/**
 * @brief __split_squeeze_in_self
 *
 *      -- squeeze space to write, , be sure there have enough sapce to squeeze before call it
 *
 * @param s   - split to squeeze
 * @param pos - direct position where to write data
 * @param cnt - the elements count wanted to write
 * @return    - the pos for write
 */
static int __split_squeeze_in_self(_split s, int pos, int cnt)
{
    //! 1. no need to move
    if(pos == _split_rpos(s))
    {
        _split_rpos(s) += cnt;

        return pos;
    }

    //! 2. no need to move
    if(pos == _split_fpos(s))
    {
        pos -= cnt;

        if(pos < 0)
        {
            _split_fpos(s) += _split_cap(s);
            _split_rpos(s) += _split_cap(s);
        }

        return _split_fpos(s);
    }

    //! 3. move prev is more wisely
    if(pos - _split_fpos(s) <= _split_rpos(s) - pos)
    {
        int target = _split_fpos(s) - cnt;

        if(target >= 0)
        {
            //!            <--| |
            //!    |*| | | |*|*|*|*|*|*|
            //!

            memmove(_split_pptr(s, target), _split_fptr(s),  cnt * _split_size(s));

            _split_fpos(s) = target;
        }
        else
        {
            //!    | |-|-|-|-|-|-| | | |
            //!   ----|               |<-|
            //!   |----------------------|

            //! first step
            memcpy (_split_fptr(s),  _split_pptr(s, _split_cap(s) + target), -target * _split_size(s));

            //! second step
            memmove(_split_pptr(s, _split_fpos(s) - target), _split_base(s), (cnt + target) * _split_size(s));

            _split_fpos(s) = _split_cap(s) + target;
            _split_rpos(s) = _split_cap(s) + _split_rpos(s);
        }
    }

    //! 4. move next is more wisely
    else
    {
        int target = _split_rpos(s) + cnt;

        if(target <= _split_cap(s))
        {
            //!               | |-->
            //!    | |-|-|-|-|-|-|-| | | | |
            //!

            memmove(_split_rptr(s), _split_pptr(s, _split_rpos(s) - cnt), cnt * _split_size(s));

            _split_rpos(s) = target;
        }
        else
        {
            //!               | |-->
            //!    | | | |-|-|-|-|-| |
            //!

            target = target % _split_cap(s);

            //! first step
            memcpy (_split_base(s), _split_pptr(s, _split_rpos(s) - target), target         * _split_size(s));

            //! second step
            memmove(_split_rptr(s), _split_pptr(s, _split_rpos(s) - cnt   ), (cnt - target) * _split_size(s));

            _split_rpos(s) = _split_cap(s) + target;
        }
    }

    return pos;
}

static void __split_squeeze_to_prev(_split s, _split f, int cnt)
{

}

static void __split_squeeze_to_next(_split s, _split f, int pos, int cnt)
{

}

/**
 * @brief __split_write_hard
 *
 *      -- write data to split, be sure there have enough sapce to write
 *
 * @param s     - split to write
 * @param in    - ptr of input data
 * @param inlen - len of input data
 * @param pos   - the direct pos to write
 * @param cnt   - element cnt to write
 */

static void __split_write_hard(_split s, int pos, constr in, int inlen, int cnt)
{
    pos = __split_squeeze_in_self(s, pos, cnt);

    if(!in)
        return;

    if(pos + cnt <= s->cap)
    {
        memcpy(_split_pptr(s, pos), in, inlen > _split_size(s) * cnt ? _split_size(s) * cnt : inlen);
    }
    else
    {
        //! first step
        int can_copy = _split_size(s) * (s->cap - pos);

        //! second step
        if(can_copy >= inlen)
        {
            memcpy(_split_pptr(s, pos), in , inlen);
        }
        else
        {
            memcpy(_split_pptr(s, pos), in           , can_copy);

            memcpy(_split_base(s)     , in + can_copy, inlen - can_copy);
        }
    }
}

static void __split_appd_hard(_split s, constr in, int cnt)
{
    __split_write_hard(s, _split_rpos(s), in, INT_MAX, cnt);
}

static int __split_appd_safe(_split s, constr in, int cnt)
{
    if(cnt > _split_avail(s))
        cnt = _split_avail(s);

    __split_appd_hard(s, in, cnt);

    return cnt;
}

static void __split_push_hard(_split s, constr in, int cnt)
{
    __split_write_hard(s, _split_fpos(s), in, INT_MAX, cnt);
}

static int __split_push_safe(_split s, constr in, int cnt)
{
    if(cnt > _split_avail(s))
        cnt = _split_avail(s);

    __split_push_hard(s, in, cnt);

    return cnt;
}

static int __split_transfer_to_prev(_split s, int cnt)
{
    int transfered;

    if(_split_fpos(s) + cnt <= _split_cap(s))
    {
        transfered = __split_appd_safe(_split_prev(s), _split_fptr(s), cnt);
        __split_fpos_shift_r(s, cnt);

        return transfered;
    }
    else
    {
        int now_transfer = _split_cap(s) - _split_fpos(s);

        //! step 1:
        transfered  = __split_appd_safe(_split_prev(s), _split_fptr(s), now_transfer);
        __split_fpos_shift_r(s, now_transfer);

        //! step 2:
        transfered += __split_transfer_to_prev(s, cnt - now_transfer);
    }

    return transfered;
}

static int __split_transfer_to_next(_split s, int cnt)
{
    int transfered;

    if(_split_rpos(s) < _split_cap(s))
    {
        transfered = __split_push_safe(_split_next(s), _split_pptr(s, -cnt), cnt);
        __split_rpos_shift_l(s, cnt);

        return transfered;
    }
    else
    {
        if(_split_rpos(s) - cnt >= _split_cap(s))
        {
            transfered = __split_push_safe(_split_next(s), _split_pptr(s, -cnt), cnt);
            __split_rpos_shift_l(s, cnt);

            return transfered;
        }
        else
        {
            int now_transfer = _split_rpos(s) - _split_cap(s);

            //! step 1:
            transfered  = __split_push_safe(_split_next(s), _split_base(s), now_transfer);
            __split_rpos_shift_l(s, cnt);

            //! step 2:
            transfered += __split_transfer_to_next(s, cnt - now_transfer);
        }
    }

    return transfered;
}

/**
 * @brief __split_squeeze_room
 *
 *
 *
 */
static bool __split_squeeze_room(_split s, int pos, int cnt, _pos info)
{
    if(_split_avail(s) >= cnt)
    {
        info->s   = s;
        info->pos = pos;

        return true;
    }
    else
    {
        int prev_avail, next_avail, prev_moves, next_moves, need_rooms;

        prev_moves = pos - _split_fpos(s);
        next_moves = _split_rpos(s) - pos;
        prev_avail = _split_prev(s) ? _split_avail(_split_prev(s)) : 0;
        next_avail = _split_next(s) ? _split_avail(_split_next(s)) : 0;

        //! not have enough space, quited
        if(cnt > _split_avail(s) + prev_avail + next_avail)
        {
            return false;
        }

        need_rooms = cnt - _split_avail(s);     // need_rooms >= 1

        if(0 == prev_moves)
        {
            if(prev_avail >= need_rooms)
            {
                info->s   = _split_prev(s);
                info->pos = _split_fpos(_split_prev(s));
                return true;
            }
            else
            {
                __split_transfer_to_next(s, need_rooms - prev_avail);
                return __split_squeeze_room(s, pos, cnt, info);
            }
        }
        else if(0 == next_moves)
        {
            if(next_avail >= need_rooms)
            {
                info->s   = s;
                info->pos = pos;

                return true;
            }
            else
            {
                __split_transfer_to_prev(s, need_rooms - next_avail);
                return __split_squeeze_room(s, pos, cnt, info);
            }
        }

        if(prev_moves <= next_moves)
        {
            need_rooms -= __split_transfer_to_prev(s, prev_moves);

            if(need_rooms)
            {
#if _EVEC_CHECK
                need_rooms -= __split_transfer_to_next(s, need_rooms);
                assert(need_rooms == 0);
#else
                __split_transfer_to_next(s, need_rooms);
#endif
            }

            return __split_squeeze_room(s, pos, cnt, info);
        }
        else
        {
            need_rooms -= __split_transfer_to_next(s, next_moves);

            if(need_rooms)
            {
#if _EVEC_CHECK
                need_rooms -= __split_transfer_to_next(s, need_rooms);
                assert(need_rooms == 0);
#else
                __split_transfer_to_next(s, need_rooms)
#endif
            }

            return __split_squeeze_room(s, pos, cnt, info);
        }
    }

    return 0;
}




#define _elist_join_prev(n, i)          \
do{                                     \
    _n_prev(i) = _n_prev(n);            \
    _n_next(i) = n;                     \
    _n_prev(n) = i;                     \
                                        \
    if(_n_prev(i))                      \
        _n_next(_n_prev(i)) = i;        \
                                        \
}while(0)

#define _elist_join_next(n, i)          \
do{                                     \
    _n_prev(i) = n;                     \
    _n_next(i) = _n_next(n);            \
    _n_next(n) = i;                     \
                                        \
    if(_n_next(i))                      \
        _n_prev(_n_next(i)) = i;        \
                                        \
}while(0)

//! compat to elist helper macros
#define _n_next(n) _split_next(n)
#define _n_prev(n) _split_prev(n)


#define _split_join_prev(n, i) _elist_join_prev(n, i)
#define _split_join_next(n, i) _elist_join_next(n, i)

/**
 * @brief __split_make_room
 *
 *      make new rooms for writen, we call this to make rooms when
 * there have no enough space to write data in cur split and closing
 * neighbour splits.
 *
 * @return (_split)-1  create a new split to write, set to s->prev
 *         (_split) 1  create a new split to write, set to s->next
 *         (_split)..  using return split to write
 */
static bool __split_make_room(_split s, int pos, int cnt, _pos info)
{
    int prev_avail, next_avail, prev_moves, next_moves, need_rooms; ;

    //! first we move out the elements from cur split to neighbours
    //!
    //! after this operation, will only have three situations:
    //!
    //!     1. pos now is at front pos or rear pos in cur split
    //!
    //!        in this case,
    //!
    //!     2. pos now is not at front pos or rear pos in cur split
    //!
    //!        in this case, both neighbours is full or not available
    //!     to wirte
    //!
    //!     3.
    //!
    //!
    //!
    {
        prev_moves = pos - _split_fpos(s);
        next_moves = _split_rpos(s) - pos;
        prev_avail = _split_prev(s) ? _split_avail(_split_prev(s)) : 0;
        next_avail = _split_next(s) ? _split_avail(_split_next(s)) : 0;

        if(prev_avail)
        {
            __split_transfer_to_prev(s, prev_moves);
            prev_avail = _split_avail(_split_prev(s));
            prev_moves = pos - _split_fpos(s);
        }

        if(next_avail)
        {
            __split_transfer_to_next(s, next_moves);
            next_avail = _split_avail(_split_next(s));
            next_moves = _split_rpos(s) - pos;
        }

        need_rooms = cnt - _split_avail(s) - prev_avail - next_avail;
    }

    //!
    //! if this split is a head or tail
    //!
    //!    step 1: try to expand it
    //!    step 2: create a new split
    //!
    {
        if(!_split_next(s) || !_split_prev(s))
        {
            if(!__split_expand(s, need_rooms, info->v->factor))
            {
                _split new_s = __split_new(need_rooms, _split_size(s));

                if(!_split_next(s))  _split_join_next(s, new_s);
                else                 _split_join_prev(s, new_s);

                return __split_squeeze_room(s, pos, cnt, info);
            }
            else
            {
                info->s   = s;
                info->pos = pos;

                return true;
            }
        }
    }

    //!
    //! now we sure this split is between head and tail
    //!
    //!     when add element to internal splits, we will expand it only if
    //! it is empty to write, else we do not try to expand it, only create
    //! new split to write
    //!
    //!     the new created split will be merged automatically in later operations
    //! when checked needed
    //!
    {
        _split new_s = __split_new(need_rooms, _split_size(s));

        if(0 == prev_avail && 0 == next_avail)
        {
            if(prev_moves < next_moves)
            {
                //! p             s        n
                //! |****|        |****r0| |****|
                //!                 ^--6
                //!
                //! p             s        n
                //! |****| |r000| |****r0| |****|       <- inserted before
                //!                 ^--6
                //!
                //! p             s        n
                //! |****| |**r0| |00**r0| |****|       <- squeezed(moved 2)
                //!           ^--6
                //!
                _split_join_prev(s, new_s);
            }
            else
            {
                //! p      s               n
                //! |****| |****r0|        |****|
                //!            ^--6
                //!
                //! p       s              n
                //! |****| |****r0| |r000| |****|       <- inserted after
                //!            ^--6
                //!
                //! p      s               n
                //! |****| |***r00| |*r00| |****|       <- squeezed(moved 1)
                //!            ^--
                //!

                _split_join_next(s, new_s);
            }

        }
        else if(0 == next_avail)
        {
            //! p           s         n
            //! |**r0|      |00***r0| |****|
            //!                ^--8
            //!
            //!             s
            //! |**r0| |r0| |00***r0| |****|            <- inserted before and squeezed(moved 1)
            //!    ^--8
            //!

            _split_join_prev(s, new_s);

            info->s   = _split_prev(s);
            info->pos = 0;

        }
        else if(0 == prev_avail)
        {
            //! p      s            n
            //! |****| |***r0|      |****|
            //!            ^--4
            //! p      s            n
            //! |****| |***r0| |r0| |****|              <- inserted after and squeezed(moved 1)
            //!            ^--4
            //!
            _split_join_next(s, new_s);
            info->s   = _split_next(s);
            info->pos = 0;
        }

        //! p      s        n
        //! |***r| |r000|   |***r|
        //!         ^--8
        //!
        //! p      s            n
        //! |***r| |r00000| |***r|
        //!     ^--8
        //!

        __split_expand(s, need_rooms, EVEC_MAX_FACTOR);

        return __split_squeeze_room(s, pos, cnt, info);
    }

    return false;
}


static cptr __pos_write(_pos pos)
{
    cptr p = _split_pptr(pos->s, pos->pos);

    if(pos->cnt <= _split_avail(pos->s))
    {
        __split_write_hard(pos->s, pos->pos, pos->in, pos->inlen, pos->cnt);
    }
    else
    {
        int write = _split_avail(pos->s);

        __split_write_hard(pos->s, pos->pos, pos->in, pos->inlen, write);

        pos->cnt   -= write;
        pos->in    += write * _split_size(pos->s);
        pos->inlen -= write * _split_size(pos->s);

        //! push to next
        __split_write_hard(_split_next(pos->s), _split_fpos(pos->s), pos->in, pos->inlen, pos->cnt);
    }

    return p;
}

static cptr __split_insert(_split s, _pos_t* pos)
{
    if(!__split_squeeze_room(s, pos->pos, pos->cnt, pos))
    {
        if(!__split_make_room(s, pos->pos, pos->cnt, pos))
            return false;
    }

    return __pos_write(pos);
}


static cptr __split_push(_split s, constr in, int inlen, int cnt)
{

}

static cptr __split_appd(_split s, constr in, int inlen, int cnt)
{

}

static cptr __split_at(_split s, int idx)
{
    if(idx > _split_len(s))
        return 0;

    return _split_pptr(s, idx);
}



/// -----------------------------------------------------------------------
//! evec basic
///

static evec __evec_new(etypev type, int size);

evec evec_new(etypev type, u16 esize)
{
    static const u8 _size_map[] = __EVAR_ITEM_LEN_MAP;

    type &= __ETYPEV_VAR_MASK;

    if(type == E_NAV)
        return 0;

    if(type != E_USER)
    {
        return __evec_new(type, _size_map[type]);
    }

    if(esize == 0)
        return 0;

    return esize > 0 ? __evec_new(E_USER, esize) : 0;
}

static evec __evec_new(etypev type, int esize)
{
    evec out = ecalloc(1, sizeof(*out));

    _v_esize (out) = esize;
    _v_factor(out) = 2;
    _v_head  (out) = __split_new(1, _v_esize(out));
    _v_tail  (out) = _v_head(out);
    _v_cap   (out) = _split_cap(_v_head(out));
    _v_splits(out) = 1;
    _v_type  (out) = type;

    return out;
}

uint   evec_len (evec v)   { return v ? _v_cnt (v)  : 0; }
uint   evec_cap (evec v)   { return v ? _v_cap (v)  : 0; }
uint   evec_esize(evec v)  { return v ? _v_esize(v) : 0; }
etypev evec_type(evec v)   { return v ? _v_type(v)  : E_NAV;}

/// ------------ evec clear and free ---------------------
///
///
///

static void __split_clear   (evec v, _split s);
static void __split_clear_ex(evec v, _split s, eobj_rls_ex_cb rls, eval prvt);
static void __split_free    (evec v, _split s);
static void __split_free_ex (evec v, _split s, eobj_rls_ex_cb rls, eval prvt);

int evec_clear  (evec v){ return evec_clearEx(v, 0, EVAL_ZORE);}
int evec_clearEx(evec v, eobj_rls_ex_cb rls, eval prvt)
{
    int cnt; _split s, next; int rsv;

    is0_ret(v, 0);

    cnt = _v_cnt(v);
    s   = _v_head(v);
    rsv = _v_reserve(v) ? _v_reserve(v) : 1;

    _v_splits(v) = 0;
    _v_tail  (v) = 0;

    if(rls)
    {
        while(s)
        {
            next = _split_next(s);

            if(rsv > 0)
            {
                __split_clear_ex(v, s, rls, prvt);

                rsv -= _split_cap(s);

                _v_splits(v)++;
                _v_tail  (v) = s;
            }
            else
            {
                if(_v_tail  (v) == _split_prev(s))
                {
                    _split_next(_split_prev(s)) = 0;
                }

                __split_free_ex(v, s, rls, prvt);
            }

            s = next;
        }
    }
    else
    {
        while(s)
        {
            next = _split_next(s);

            if(rsv > 0)
            {
                __split_clear(v, s);

                rsv -= _split_cap(s);

                _v_splits(v)++;
                _v_tail  (v) = s;
            }
            else
            {
                if(_v_tail  (v) == _split_prev(s))
                {
                    _split_next(_split_prev(s)) = 0;
                }

                __split_free(v, s);
            }

            s = next;
        }
    }

    return cnt;
}

int  evec_free  (evec v){ return evec_freeEx(v, 0, EVAL_ZORE); }
int  evec_freeEx(evec v, eobj_rls_ex_cb rls, eval prvt)
{
    int cnt; _split s, next;

    is0_ret(v, 0);

    cnt = _v_cnt(v);

    s = _v_head(v);

    if(rls)
    {
        while(s)
        {
            next = _split_next(s);

            __split_free_ex(v, s, rls, prvt);

            s = next;
        }
    }
    else
    {
        while(s)
        {
            next = _split_next(s);

            __split_free(v, s);

            s = next;
        }
    }

    efree(v);

    return cnt ? cnt : -1;
}

static void __split_clear(evec v, _split s)
{
    switch (_v_type(v))
    {
        case E_STR:
        case E_RAW: {
                        int pos;

                        for(pos = _split_fpos(s) ; pos < _split_rpos(s); pos++)
                        {
                            _s_free( *(estr*)_split_pptr(s, pos) );
                        }
                    }
                    break;
    }

    if(_split_rpos(s) >= _split_cap(s))
    {
        memset(_split_fptr(s), 0, _split_eptr(s) - _split_fptr(s));
        memset(_split_base(s), 0, _split_rptr(s) - _split_base(s));
    }
    else
    {
        memset(_split_fptr(s), 0, _split_avail(s) * _split_size(s));
    }

    _split_fpos(s) = 0;
    _split_rpos(s) = 0;
}

static void __split_clear_ex (evec v, _split s, eobj_rls_ex_cb rls, eval prvt)
{
    switch (_v_type(v))
    {
        case E_STR: {
                        int pos;

                        for(pos = _split_fpos(s) ; pos < _split_rpos(s); pos++)
                        {
                            _s_free( *(estr*)_split_pptr(s, pos) );
                        }
                    }
                    break;

        case E_RAW: {
                        int pos; _enode_t b;

                        _ehdr_type_oe(b.hdr) = _ERAW_P;

                        for(pos = _split_fpos(s) ; pos < _split_rpos(s); pos++)
                        {
                            b.obj.s    = *(estr*)_split_pptr(s, pos);
                            b.hdr._len = _s_len(b.obj.s);

                            rls(&b.obj, prvt);

                            _s_free( b.obj.s );
                        }
                    }
                    break;

        case E_USER:{
                        int pos; _enode_t b;

                        _ehdr_type_oe(b.hdr) = _ERAW_P;

                        for(pos = _split_fpos(s) ; pos < _split_rpos(s); pos++)
                        {
                            b.obj.s    = _split_pptr(s, pos);
                            b.hdr._len = _s_len(b.obj.s);

                            rls(&b.obj, prvt);
                        }
                    }
                    break;
    }

    if(_split_rpos(s) >= _split_cap(s))
    {
        memset(_split_fptr(s), 0, _split_eptr(s) - _split_fptr(s));
        memset(_split_base(s), 0, _split_rptr(s) - _split_base(s));
    }
    else
    {
        memset(_split_fptr(s), 0, _split_avail(s) * _split_size(s));
    }

    _split_fpos(s) = 0;
    _split_rpos(s) = 0;
}

static void __split_free(evec v, _split s)
{
    switch (_v_type(v))
    {
        case E_STR:
        case E_RAW: {
                        int pos;

                        for(pos = _split_fpos(s) ; pos < _split_rpos(s); pos++)
                        {
                            _s_free( *(estr*)_split_pptr(s, pos) );
                        }
                    }
                    break;
    }

    efree(_split_base(s));
    efree(s);
}

static void __split_free_ex(evec v, _split s, eobj_rls_ex_cb rls, eval prvt)
{
    switch (_v_type(v))
    {
        case E_STR: {
                        int pos;

                        for(pos = _split_fpos(s) ; pos < _split_rpos(s); pos++)
                        {
                            _s_free( *(estr*)_split_pptr(s, pos) );
                        }
                    }
                    break;

        case E_RAW: {
                        int pos; _enode_t b;

                        _ehdr_type_oe(b.hdr) = _ERAW_P;

                        for(pos = _split_fpos(s) ; pos < _split_rpos(s); pos++)
                        {
                            b.obj.s    = *(estr*)_split_pptr(s, pos);
                            b.hdr._len = _s_len(b.obj.s);

                            rls(&b.obj, prvt);

                            _s_free( b.obj.s );
                        }
                    }
                    break;

        case E_USER:{
                        int pos; _enode_t b;

                        _ehdr_type_oe(b.hdr) = _ERAW_P;

                        for(pos = _split_fpos(s) ; pos < _split_rpos(s); pos++)
                        {
                            b.obj.s    = _split_pptr(s, pos);
                            b.hdr._len = _s_len(b.obj.s);

                            rls(&b.obj, prvt);
                        }
                    }
                    break;
    }

    efree(_split_base(s));
    efree(s);
}

/** -----------------------------------------------------
 *  -- evec add --
 *
 */
static void __evec_get_pos(evec v, uint idx, _pos p  );
static bool __evec_addV   (evec v, uint idx, evar var);

bool evec_pushV(evec v, evar   var) { return __evec_addV(v, 0       ,          var ); }
bool evec_pushI(evec v, i64    val) { return __evec_addV(v, 0       , EVAR_I64(val)); }
bool evec_pushF(evec v, f64    val) { return __evec_addV(v, 0       , EVAR_F64(val)); }
bool evec_pushS(evec v, constr str) { return __evec_addV(v, 0       , EVAR_CS (str)); }
bool evec_pushP(evec v, conptr ptr) { return __evec_addV(v, 0       , EVAR_CP (ptr)); }
bool evec_pushR(evec v, uint   len) { return __evec_addV(v, 0       , EVAR_RAW(0, len)); }

bool evec_appdV(evec v, evar   var) { return __evec_addV(v, UINT_MAX,          var ); }
bool evec_appdI(evec v, i64    val) { return __evec_addV(v, UINT_MAX, EVAR_I64(val)); }
bool evec_appdF(evec v, f64    val) { return __evec_addV(v, UINT_MAX, EVAR_F64(val)); }
bool evec_appdS(evec v, constr str) { return __evec_addV(v, UINT_MAX, EVAR_CS (str)); }
bool evec_appdP(evec v, conptr ptr) { return __evec_addV(v, UINT_MAX, EVAR_CP (ptr)); }
bool evec_appdR(evec v, uint   len) { return __evec_addV(v, UINT_MAX, EVAR_RAW(0, len)); }

bool evec_addV(evec v, uint i, evar   var) { return __evec_addV(v, i,          var ); }
bool evec_addI(evec v, uint i, i64    val) { return __evec_addV(v, i, EVAR_I64(val)); }
bool evec_addF(evec v, uint i, f64    val) { return __evec_addV(v, i, EVAR_F64(val)); }
bool evec_addS(evec v, uint i, constr str) { return __evec_addV(v, i, EVAR_CS (str)); }
bool evec_addP(evec v, uint i, conptr ptr) { return __evec_addV(v, i, EVAR_CP (ptr)); }
bool evec_addR(evec v, uint i, uint   len) { return __evec_addV(v, i, EVAR_RAW(0, len)); }

static bool __evec_addB(evec v, uint idx, constr in, uint len, int cnt)
{
    _pos_t p = {v, in, len, cnt, 0, 0};

    _v_cnt(v) += cnt;

    if(idx == 0)
    {
        p.s   = _v_head(v);
        p.pos = _split_fpos(p.s);

    }
    else if(idx >= _v_cnt(v))
    {
        p.s = _v_tail(v);
        p.pos = _split_rpos(p.s);
    }
    else
        __evec_get_pos(v, idx, &p);

    return __split_insert(p.s, &p);
}

static bool __evec_addV(evec v, uint idx, evar var)
{
    int v_type;

    is1_ret(!v, false);

    v_type = var.type & __ETYPEV_VAR_MASK;

    switch (_v_type(v))
    {
        case E_USER :   if(v_type != E_RAW)
                            return false;

                        return var.type & __ETYPEV_PTR_MASK ? __evec_addB(v, idx, var.v.p, var.esize, 1)
                                                            : __evec_addB(v, idx, var.v.r, var.esize, 1);

        default     :   if(v_type != _v_type(v))
                            return false;

                        switch(v_type)
                        {
                            case E_STR :    var.v.s = estr_dupS(var.v.s  );
                                            break;

                            case E_RAW :    var.v.p   = estr_newLen(var.v.p, var.esize);
                                            var.esize = 8;
                                            break;
                        }

                        break;

    }

    return __evec_addB(v, idx, var.v.r, var.esize, var.cnt ? var.cnt : 1);
}

static void __evec_get_pos(evec v, uint idx, _pos p)
{
    p->s = _v_head(v);

    do{
        //! found
        //!

        uint cur_len = _split_len(p->s);

        if(cur_len > idx)
        {
            p->pos = idx;

            break;
        }

        idx -= _split_len(p->s);

        p->s = _split_next(p->s);
    }while(1);
}

/// -----------------------------------------------------
/// evec val
///
///

cptr evec_val (evec v, uint idx){ _pos_t p;is1_ret(!v || idx >= _v_cnt(v)                                               , 0); __evec_get_pos(v, idx, &p); return _split_pptr(p.s, p.pos);}
i64  evec_valI(evec v, uint idx){ _pos_t p;is1_ret(!v || idx >= _v_cnt(v) ||(_v_type(v) != E_I64 && _v_type(v) != E_F64), 0); __evec_get_pos(v, idx, &p); return _v_type(v) == E_I64 ? *(i64*)_split_pptr(p.s, p.pos) : *(f64*)_split_pptr(p.s, p.pos);}
f64  evec_valF(evec v, uint idx){ _pos_t p;is1_ret(!v || idx >= _v_cnt(v) ||(_v_type(v) != E_I64 && _v_type(v) != E_F64), 0); __evec_get_pos(v, idx, &p); return _v_type(v) == E_I64 ? *(i64*)_split_pptr(p.s, p.pos):  *(f64*)_split_pptr(p.s, p.pos);}
cstr evec_valS(evec v, uint idx){ _pos_t p;is1_ret(!v || idx >= _v_cnt(v) || _v_type(v) != E_STR                        , 0); __evec_get_pos(v, idx, &p); return *(cstr*)_split_pptr(p.s, p.pos);}
cptr evec_valP(evec v, uint idx){ _pos_t p;is1_ret(!v || idx >= _v_cnt(v) || _v_type(v) != E_PTR                        , 0); __evec_get_pos(v, idx, &p); return *(cptr*)_split_pptr(p.s, p.pos);}
cptr evec_valR(evec v, uint idx){ _pos_t p;is1_ret(!v || idx >= _v_cnt(v) || _v_type(v) != E_RAW                        , 0); __evec_get_pos(v, idx, &p); return *(cptr*)_split_pptr(p.s, p.pos);}

/// -----------------------------------------------------
/// evec take
///
///



