// =====================================================================================
//
//       Filename:  esl.h
//
//    Description:  easy skiplist - rebuild from redis
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

#ifndef __ESL_H__
#define __ESL_H__

#define ESL_VERSION "esl 1.0.9"     // adjust return val of esl_free*()

#include "etype.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESL_MAXLEVEL 16       // Should be enough for 2^16 elements
#define ESL_P        0.25     // Skiplist P = 1/4

enum {
    ESL_ASC = 0x00,    // Ascending
    ESL_DES = 0x01,    // Descending
};

typedef struct eskiplist_s* esl;

typedef struct eskiplist_node_s{
    i64   score;
    cptr  obj;
}* esln;

esl  esl_new(int opts);

void esl_free (esl sl);                       // free the esl  list
cptr esl_freeP(esl sl, i64 key, cptr obj );   // free the esl  node who handle this obj, return the hold obj if succeed
cptr esl_freeN(esl sl, i64 key, esln node);   // free the esl  node, return the hold obj if succeed
cptr esl_freeH(esl sl);                       // free the head node, return the hold obj if succeed
cptr esl_freeT(esl sl);                       // free the tail node, return the hold obj if succeed

esln esl_addP(esl sl, i64 key, cptr ptr );
esln esl_addN(esl sl, i64 key, esln node);     // the node to add must not in a eskiplist

esln esl_find(esl sl, i64 key);

esln esl_first(esl  sl);
esln esl_last (esl  sl);
esln esl_next (esln node);
esln esl_prev (esln node);
#define esl_itr2(sl, itr, tmp) for(tmp = esl_first(sl); (itr = tmp, tmp = esl_next(tmp), itr); )

esln esl_rmO(esl sl, i64 key, cptr obj );
esln esl_rmN(esl sl, i64 key, esln node);

esln esl_popH(esl sl);
esln esl_popT(esl sl);

uint esl_len(esl sl);


#ifdef __cplusplus
}
#endif

#endif
