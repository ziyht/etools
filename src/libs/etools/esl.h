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

#define ESL_VERSION "esl 1.0.1"     // expose score to public in esln

#include "etype.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESKIPLIST_MAXLEVEL 16       // Should be enough for 2^16 elements
#define ESKIPLIST_P        0.25     // Skiplist P = 1/4

typedef struct eskiplist_s* esl;

typedef struct eskiplist_node_s{
    s64   score;
    cptr  obj;
}* esln;

esl  esl_new(int safe);

void esl_free (esl sl);
int  esl_freeO(esl sl, s64 score, cptr obj );
int  esl_freeN(esl sl, s64 score, esln node);
int  esl_freeH(esl sl);
int  esl_freeT(esl sl);

esln esl_insertO(esl sl, s64 score, cptr obj );
esln esl_insertN(esl sl, s64 score, esln node);     // the node to add must not in a eskiplist

esln esl_first(esl  sl);
esln esl_last (esl  sl);
esln esln_next(esln node);
esln esln_prev(esln node);
#define esl_itr2(sl, itr, tmp) for(tmp = esl_first(sl); (itr = tmp, tmp = esln_next(tmp), itr); )

esln esl_rmO(esl sl, s64 score, cptr obj );
esln esl_rmN(esl sl, s64 score, esln node);

esln esl_popH(esl sl);
esln esl_popT(esl sl);

uint esl_len(esl sl);


#ifdef __cplusplus
}
#endif

#endif
