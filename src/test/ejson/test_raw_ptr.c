#include "test.h"

void ejson_raw_ptr_test()
{
    ejson e; ejsw w;

    e = ejso_new(_OBJ_);
    w = ejsw_new(8);


    ejso_addR(e, "raw1", 100);
    ejso_addR(e, "raw2", 100);
    ejso_addR(e, "raw3", 100);
    ejso_addP(e, "e", e);
    ejso_addP(e, "w", w);
    puts("before set:");
    ejso_toFWra(e, w); ejsw_show(w);
    puts("");

    ejsk_setR(e, "raw1", 50);
    ejsk_setR(e, "raw2", 100);
    ejsk_setR(e, "raw3", 150);
    puts("raw1 -> 50 raw3 -> 150");
    ejso_toFWra(e, w); ejsw_show(w);
    puts("");


    ejso_free(e);
    ejsw_free(w);
}
