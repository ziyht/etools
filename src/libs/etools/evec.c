#include <string.h>
#include <assert.h>

#include "ealloc.h"
#include "etype.h"
#include "eutils.h"

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
    _split      s;      // split
    int         pos;    // postion (direct)
    uint        f;      // factor
    evec        hd;     // evec handle
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

#define _split_fptr(s)          _split_base(s) + (_split_fpos(s) * _split_size(s))         // ptr of front (first element)
#define _split_rptr(s)          _split_base(s) + (_split_rpos(s) * _split_size(s))         // ptr of rear
#define _split_lptr(s)          _split_base(s) + (((_split_rear(s) - 1) % _split_cap(s)) * _split_size(s)) // ptr of last element
#define _split_eptr(s)          _split_base(s) + (_split_cap(s) * _split_size(s))          // end ptr of base

#define _split_pptr(s, p)       _split_base(s) + (p) * _split_size(s)

#pragma pack(1)

typedef struct evec_s
{
    _split      head;
    _split      tail;

    u32         cnt;                // all element count
    u32         cap;                // all buffer  cap

    u32         size;               // element size
    u32         splits;             // splits count

    u32         factor;             // factor for create, expand, shrink, merging, delete splits operations
    u32         reserves;           //

    uint        type         : 8;   // etypev
    uint        need_merging : 1;
}evec_t;

#pragma pack()

#define _v_head(v)          (v)->head
#define _v_tail(v)          (v)->tail
#define _v_cnt(v)           (v)->cnt
#define _v_cap(v)           (v)->cap
#define _v_size(v)          (v)->size
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
    out->size = size;
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
            if(!__split_expand(s, need_rooms, info->f))
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


static cptr __pos_write(_pos info, constr in, int inlen, int cnt)
{
    cptr p = _split_pptr(info->s, info->pos);

    if(cnt <= _split_avail(info->s))
    {
        __split_write_hard(info->s, info->pos, in, inlen, cnt);
    }
    else
    {
        int write = _split_avail(info->s);

        __split_write_hard(info->s, info->pos, in, inlen, write);

        cnt   -= write;
        in    += write * _split_size(info->s);
        inlen -= write * _split_size(info->s);

        //! push to next
        __split_write_hard(_split_next(info->s), _split_fpos(info->s), in, inlen, cnt);
    }

    return p;
}

static cptr __split_insert(_split s, constr in, int inlen, int pos, int cnt)
{
    _pos_t info;

    if(!__split_squeeze_room(s, pos, cnt, &info))
    {
        if(!__split_make_room(s, pos, cnt, &info))
            return false;
    }

    return __pos_write(&info, in, inlen, cnt);
}


static cptr __split_push(_split s, constr in, int inlen)
{
    return __split_insert(s, in, inlen, _split_fpos(s), 1);
}

static cptr __split_appd(_split s, constr in, int inlen)
{
    return __split_insert(s, in, inlen, _split_rpos(s), 1);
}

static cptr __split_at(_split s, int idx)
{
    if(idx > _split_len(s))
        return 0;

    return _split_pptr(s, idx);
}



//! -----------------------------------------------------------------------
//!  evec

/** -----------------------------------------------------
 *
 *  evec new
 *
 * ------------------------------------------------------
 */
static evec __evec_new(etypev type, int size);

evec evec_new(etypev type)
{
    static const u8 _size_map[] = __EVAR_ITEM_LEN_MAP;

    type &= __ETYPEV_VAR_MASK;

    return type >= E_USER ? 0 : __evec_new(type, _size_map[type]);
}

evec evec_newEx(int size)
{
    return __evec_new(E_USER, size);
}

static evec __evec_new(etypev type, int size)
{
    evec out = ecalloc(1, sizeof(*out));

    _v_size  (out) = size;
    _v_factor(out) = 2;
    _v_head  (out) = __split_new(1, _v_size(out));
    _v_tail  (out) = _v_head(out);
    _v_cap   (out) = _split_cap(_v_head(out));
    _v_splits(out) = 1;
    _v_type  (out) = type;

    return out;
}

uint evec_len (evec v)   { return v ? _v_cnt (v) : 0; }
uint evec_cap (evec v)   { return v ? _v_cap (v) : 0; }
uint evec_size(evec v)   { return v ? _v_size(v) : 0; }

/** -----------------------------------------------------
 *
 *  evec clear
 *
 * ------------------------------------------------------
 */
static void __split_clear     (_split s);
static void __split_clear_ex  (_split s, eobj_rls_ex_cb rls, eval prvt);
static void __split_destroy   (_split s);
static void __split_destroy_ex(_split s, eobj_rls_ex_cb rls, eval prvt);

int evec_clear  (evec v){ return evec_clearEx(v, 0, (eval){0});}
int evec_clearEx(evec v, eobj_rls_ex_cb rls, eval prvt)
{
    int cnt; _split s, tmp;

    is0_ret(v, 0);

    cnt = _v_cnt(v);

    s = _v_head(v);

    if(rls)
    {
        while(s)
        {
            tmp = _split_next(s);
            __split_destroy_ex(s, rls, prvt);
            s = tmp;
        }
    }
    else
    {
        while(s)
        {
            tmp = _split_next(s);
            __split_destroy(s);
            s = tmp;
        }
    }

    return cnt;
}

static void __split_clear(_split s)
{
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

static void __split_destroy(_split s)
{
    efree(_split_base(s));
    efree(s);
}


static void __split_destroy_ex(_split s, eobj_rls_ex_cb rls, eval prvt)
{
    __split_destroy(s);
}

/** -----------------------------------------------------
 *
 *  evec push appd add(insert)
 *
 * ------------------------------------------------------
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

static bool __evec_addV(evec v, uint idx, evar var)
{
    _pos_t info;

    var.type &= __ETYPEV_VAR_MASK;

    is1_ret(!v || var.type != _v_type(v), false);

    if(var.type > E_PTR)
    {
        switch (var.type)
        {
            case E_STR : var.v.s = strdup (var.v.s  ); break;

            case E_RAW : {
                             char* buf = emalloc(var.size);
                             if(var.v.p)
                             {
                                 memcpy(buf, var.v.p, var.size);
                             }
                             var.v.p = buf;
                         }
                         break;

            default    : break;
        }
    }

    if(idx == 0)
    {
        return __split_push(_v_head(v), var.v.r, _v_size(v));
    }
    else if(idx >= _v_cnt(v))
    {
        return __split_appd(_v_tail(v), var.v.r, _v_size(v));
    }

    __evec_get_pos(v, idx, &info);

    return __split_insert(info.s, var.v.r, _v_size(v), info.pos, 1);
}

static void __evec_get_pos(evec v, uint idx, _pos p)
{
    if(idx > _v_cnt(v))
        return;

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
