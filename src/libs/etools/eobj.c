#include "_eobj_header.h"

#include "ell.h"
#include "esl.h"
#include "edict.h"
#include "erb.h"

#include "eutils.h"

int eobj_free(eobj o)
{
    is1_ret(!o || _eo_linked(o), 0);

    if(eobj_typeo(o) == EOBJ)
    {
        switch(_eo_typec(o))
        {
            case ELL  : return ell_free  ((ell  )o);
            case EDICT: return edict_free((edict)o);
            case ERB  : return erb_free  ((erb  )o);
            case ESL  : return esl_free  ((esl  )o);
        }
    }
    else
    {

    }

    return 0;
}

eobj eobj_setKeyI(eobj o, i64 key)
{
    if(o)
    {
        is1_ret(_eo_linked(o), 0);

        switch (_eo_typec(o)) {
            case ERB    :
            case EDICT  :
            case ESL    : if(_eo_keys(o)) _eobj_free(_eo_keyS(o));

                          _eo_typek(o) = _EO_KEYI;
                          _eo_keyI(o)  = key;

            default     : return 0;
        }
    }

    return o;
}

eobj eobj_setKeyS(eobj o, constr key)
{
    if(o)
    {
        is1_ret(_eo_linked(o), 0);

        switch (_eo_typec(o)) {
            case ERB    :
            case EDICT  :
            case ESL    : if(_eo_keys(o)) _eobj_free(_eo_keyS(o));

                          _eo_typek(o) = _EO_KEYS;
                          _eo_keyS(o)  = strdup(key);

            default     : return 0;
        }
    }

    return o;
}

constr __eobj_typeS(eobj o, bool beauty)
{
    #define SDS_STACK_MASK 8    // form estr.c

    typedef struct _inner_{
        uint cap;
        uint len;
        u8   type;
        char s[16];
    }_inner_;

    static _inner_ b_nil    = {5, 5, SDS_STACK_MASK, "ENIL    "};
    static _inner_ b_false  = {5, 5, SDS_STACK_MASK, "EFALSE  "};
    static _inner_ b_true   = {4, 4, SDS_STACK_MASK, "ETRUE   "};
    static _inner_ b_null   = {4, 4, SDS_STACK_MASK, "ENULL   "};
    static _inner_ b_num    = {6, 6, SDS_STACK_MASK, "ENUM    "};
    static _inner_ b_str    = {6, 6, SDS_STACK_MASK, "ESTR    "};
    static _inner_ b_ptr    = {5, 5, SDS_STACK_MASK, "EPTR    "};
    static _inner_ b_raw    = {5, 5, SDS_STACK_MASK, "ERAW    "};
    static _inner_ b_obj    = {3, 3, SDS_STACK_MASK, "EOBJ    "};
    static _inner_ b_arr    = {5, 5, SDS_STACK_MASK, "EARR    "};
    static _inner_ b_unknown= {6, 6, SDS_STACK_MASK, "EUNKNOWN"};

    static _inner_ t_nil    = {5, 5, SDS_STACK_MASK, "ENIL"    };
    static _inner_ t_false  = {5, 5, SDS_STACK_MASK, "EFALSE"  };
    static _inner_ t_true   = {4, 4, SDS_STACK_MASK, "ETRUE"   };
    static _inner_ t_null   = {4, 4, SDS_STACK_MASK, "ENULL"   };
    static _inner_ t_num    = {6, 6, SDS_STACK_MASK, "ENUM"    };
    static _inner_ t_str    = {6, 6, SDS_STACK_MASK, "ESTR"    };
    static _inner_ t_ptr    = {5, 5, SDS_STACK_MASK, "EPTR"    };
    static _inner_ t_raw    = {5, 5, SDS_STACK_MASK, "ERAW"    };
    static _inner_ t_obj    = {3, 3, SDS_STACK_MASK, "EOBJ"    };
    static _inner_ t_arr    = {5, 5, SDS_STACK_MASK, "EARR"    };
    static _inner_ t_unknown= {6, 6, SDS_STACK_MASK, "EUNKNOWN"};

    if(beauty)
    {
        if(!o) return b_nil.s;

        switch (_eo_typeo(o))
        {
            case _EFALSE : return b_false  .s ;
            case _ETRUE  : return b_true   .s ;
            case _ENULL  : return b_null   .s ;
            case _ENUM   : return b_num    .s ;
            case _ESTR   : return b_str    .s ;
            case _EPTR   : return b_ptr    .s ;
            case _ERAW   : return b_raw    .s ;
            case _EOBJ   : return b_obj    .s ;
            case _EARR   : return b_arr    .s ;
            default      : return b_unknown.s ;
        }
    }
    else
    {
        if(!o) return t_nil.s;

        switch (_eo_typeo(o))
        {
            case _EFALSE : return t_false  .s ;
            case _ETRUE  : return t_true   .s ;
            case _ENULL  : return t_null   .s ;
            case _ENUM   : return t_num    .s ;
            case _ESTR   : return t_str    .s ;
            case _EPTR   : return t_ptr    .s ;
            case _ERAW   : return t_raw    .s ;
            case _EOBJ   : return t_obj    .s ;
            case _EARR   : return t_arr    .s ;
            default      : return t_unknown.s ;
        }
    }

    return "";
}

constr eobj_typeoS(eobj o)
{
    return __eobj_typeS(o, 1);
}
