//
// Created by ziyht on 17-2-8.
//

#include "test.h"



void ejson_sub_test()
{
    ejson e = ejso_new(_OBJ_); ejsw w = ejsw_new(0);

    ejson e0, e1, e2, e3;

    printf("--------- ejson_sub_test ------------\n"); fflush(stdout);

    e0 = ejso_addS(e, "0", "abcdasd");
    e1 = ejso_addS(e, "1", "abcd${PATH}asdasdf");
    e2 = ejso_addS(e, "2", "abcd${PATH}asdasdf${PATH}");
    e3 = ejso_addS(e, "3", "abcd${PATH}asdasdf${PATH}.../sdf///${PATH}fd%asd");
    printf("src: \n%s\n", ejso_toFWra(e, w)); fflush(stdout);

    ejso_subS(e0, "${PATH}", "${}");
    ejso_subS(e1, "${PATH}", "${}");
    ejso_subS(e2, "${PATH}", "${}");
    ejso_subS(e3, "${PATH}", "${}");
    printf("\"${PATH}\" -> \"${}\":\n%s\n", ejso_toFWra(e, w)); fflush(stdout);

    ejso_subS(e0, "${}", "${PATH}");
    ejso_subS(e1, "${}", "${PATH}");
    ejso_subS(e2, "${}", "${PATH}");
    ejso_subS(e3, "${}", "${PATH}");
    printf(":\"${}\" -> \"${PATH}\":\n%s\n", ejso_toFWra(e, w)); fflush(stdout);

    ejso_subS(e0, "${PATH}", "${abcd}");
    ejso_subS(e1, "${PATH}", "${abcd}");
    ejso_subS(e2, "${PATH}", "${abcd}");
    ejso_subS(e3, "${PATH}", "${abcd}");
    printf(":\"${PATH}\" -> \"${abcd}\":\n%s\n", ejso_toFWra(e, w)); fflush(stdout);

    ejsk_subS(e, "0", "${abcd}", 0);
    ejsk_subS(e, "1", "${abcd}", 0);
    ejsk_subS(e, "2", "${abcd}", 0);
    ejsk_subS(e, "3", "${abcd}", 0);
    printf("\"${abcd}\" -> \"\":\n%s\n", ejso_toFWra(e, w)); fflush(stdout);

    ejso_free(e);
    ejsw_free(w);
}
