// =====================================================================================
//
//       Filename:  esl.c
//
//    Description:  easy skiplist
//
//        Version:  1.0
//        Created:  03/08/2017 02:55:34 PM
//       Revision:  none
//       Compiler:  gcc
//
//         Author:  Haitao Yang, joyhaitao@foxmail.com
//        Company:
//
// =====================================================================================

#include <stdlib.h>

#include "esl.h"

#define   COMPAT_THREAD
#include "compat.h"

typedef struct _eskiplist_node_s* _esln;
typedef struct _eskiplist_node_s {
    s64   score;
    cptr  obj;

    _esln backward;
    _esln forward[];
} _esln_t;

typedef struct eskiplist_s {
    _esln header;
    _esln tail;
    uint  length;
    uint  level    : 16;
    uint  safe     : 1;
    uint  free     : 1;
    uint  reserved : 14;

    mutex_t mu;
}esl_t;

#define lock_sl(sl) if((sl)->safe){mutex_lock(sl->mu);}
#define ulck_sl(sl) if((sl)->safe){mutex_ulck(sl->mu);}

static _esln _esl_create_node(int level, s64 score, cptr obj) {
    _esln zn  = (_esln)malloc(sizeof(*zn) + level * sizeof(_esln));
    zn->score = score;
    zn->obj   = obj;
    return zn;
}

esl  esl_new(int safe)
{
    int j; esl sl;

    sl = safe ? (esl)malloc(sizeof(*sl))
              : (esl)malloc(sizeof(*sl) - sizeof(mutex_t));
    sl->level  = 1;
    sl->length = 0;
    sl->header = _esl_create_node(ESKIPLIST_MAXLEVEL, 0, NULL);
    for (j = 0; j < ESKIPLIST_MAXLEVEL; j++) {
        sl->header->forward[j] = NULL;
    }
    sl->header->backward = NULL;
    sl->tail = NULL;

    if(safe)
    {
        mutex_init(sl->mu);
        sl->safe = 1;
    }
    else
        sl->safe = 0;


    return sl;
}

static void _esl_free_node(_esln node) {
    free(node);
}

void esl_free(esl sl)
{
    _esln node, next;

    lock_sl(sl);
    sl->free = 1;

    node = sl->header->forward[0];

    free(sl->header);
    while(node) {
        next = node->forward[0];
        _esl_free_node(node);
        node = next;
    }

    ulck_sl(sl);
    free(sl);
}

/* Internal function used by zslDelete, zslDeleteByScore and zslDeleteByRank */
static void _esl_delete_node(esl sl, _esln x, _esln* update) {
    int i;

    for (i = 0; i < sl->level; i++) {
        if (update[i]->forward[i] == x) {
            update[i]->forward[i] = x->forward[i];
        }
    }
    if (x->forward[0]) {
        x->forward[0]->backward = x->backward;
    } else {
        sl->tail = x->backward;
    }
    while(sl->level > 1 && sl->header->forward[sl->level-1] == NULL)
        sl->level--;
    sl->length--;
}

int  esl_freeO(esl sl, s64 score, cptr obj )
{
    _esln x = (_esln)esl_rmO(sl, score, obj);

    if(x)
    {
        _esl_free_node(x);
        return 1;
    }

    return 0;
}

int  esl_freeN(esl sl, s64 score, esln node)
{
    _esln x = (_esln)esl_rmN(sl, score, node);

    if(x)
    {
        _esl_free_node(x);
        return 1;
    }

    return 0;
}

int  esl_freeH(esl sl)
{
    _esln x = (_esln)esl_popH(sl);

    if(x)
    {
        _esl_free_node(x);
        return 1;
    }

    return 0;
}

int  esl_freeT(esl sl);

static int _esl_random_level() {
    int level = 1;
#if _WIN32
#define random rand
#endif
    while ((random()&0xFFFF) < (ESKIPLIST_P * 0xFFFF))
        level += 1;
    return (level < ESKIPLIST_MAXLEVEL) ? level : ESKIPLIST_MAXLEVEL;
}

esln esl_insertO(esl sl, s64 score, cptr obj)
{
    _esln update[ESKIPLIST_MAXLEVEL], x; int i, level;

    x = sl->header;
    for (i = sl->level - 1; i >= 0; i--)
    {
        while (x->forward[i] && (x->forward[i]->score < score || (x->forward[i]->score == score && x->forward[i]->obj < obj)))
            x = x->forward[i];

        update[i] = x;
    }

    /* we assume the key is not already inside, since we allow duplicated
    * scores, and the re-insertion of score and redis object should never
    * happpen since the caller of zslInsert() should test in the hash table
    * if the element is already inside or not. */
    level = _esl_random_level();
    if (level > sl->level)
    {
        for (i = sl->level; i < level; i++)
            update[i] = sl->header;

        sl->level = level;
    }
    x = _esl_create_node(level, score, obj);
    for (i = 0; i < level; i++)
    {
        x->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = x;
    }

    x->backward = (update[0] == sl->header) ? NULL : update[0];
    if (x->forward[0])
        x->forward[0]->backward = x;
    else
        sl->tail = x;
    sl->length++;
    return (esln)x;
}



esln esl_first (esl sl)
{
    return (esln)sl->header->forward[0];
}

esln esl_last (esl  sl)
{
    return (esln)sl->tail;
}

esln esln_next(esln node)
{
    return node ? (esln)((_esln)node)->forward[0] : 0;
}
esln esln_prev(esln node)
{
    return node ? (esln)((_esln)node)->backward : 0;
}

esln esl_rmO(esl sl, s64 score, cptr obj )
{
    _esln update[ESKIPLIST_MAXLEVEL], x; int i;

    x = sl->header;
    for (i = sl->level-1; i >= 0; i--) {
        while (x->forward[i] && (x->forward[i]->score < score || (x->forward[i]->score == score && x->forward[i]->obj < obj)))
            x = x->forward[i];
        update[i] = x;
    }
    /* We may have multiple elements with the same score, what we need
    * is to find the element with both the right score and object. */
    x = x->forward[0];
    if (x && score == x->score && x->obj == obj) {
        _esl_delete_node(sl, x, update);
        return (esln)x;
    } else {
        return 0; /* not found */
    }
}

esln esl_rmN(esl sl, s64 score, esln node)
{
    _esln update[ESKIPLIST_MAXLEVEL], x; int i;

    x = sl->header;
    for (i = sl->level-1; i >= 0; i--) {
        while (x->forward[i] && (x->forward[i]->score < score || (x->forward[i]->score == score && x->forward[i]->obj < node->obj)))
            x = x->forward[i];
        update[i] = x;
    }
    /* We may have multiple elements with the same score, what we need
    * is to find the element with both the right score and object. */
    x = x->forward[0];
    if (x && score == x->score && (esln)x == node) {
        _esl_delete_node(sl, x, update);
        return (esln)x;
    } else {
        return 0; /* not found */
    }
}

esln esl_popH(esl sl)
{
    int i; _esln x;

    x = sl->header->forward[0];
    if (!x) {
        return 0; /* empty list*/
    }

    for (i = 0; i < sl->level; i++) {
        if (sl->header->forward[i] == x) {
            sl->header->forward[i] = x->forward[i];
        }
    }
    if (x->forward[0]) {
        x->forward[0]->backward = x->backward;
    } else {
        sl->tail = x->backward;
    }
    while(sl->level > 1 && sl->header->forward[sl->level - 1] == NULL)
        sl->level--;
    sl->length--;

    return (esln)x;
}

esln esl_popT(esl sl);

uint esl_len(esl sl)
{
    return sl ? sl->length : 0;
}


