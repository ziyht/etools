#include "test.h"


int main()
{
    ejson_check_test();

    ejson_cJSON_test();

    ejson_eval_test();
    ejson_obj_add_test();
    ejson_obj_get_test();
    ejson_del_test();

    ejson_sub_test();

    ejson_sort_test();

    // ejson_performance_test();

    ejson_raw_ptr_test();

    printf("ejson version(%d): %s\n", ejss_len(ejson_version()), ejson_version()); fflush(stdout);
}
