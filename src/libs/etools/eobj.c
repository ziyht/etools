#include "_eobj_header.h"
#include "eutils.h"

#include "eobj.h"

int eobj_free(eobj o)
{
    is1_ret(!o || _eo_linked(o), 0);

    if(eobj_typeo(o) == EOBJ)
    {

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
