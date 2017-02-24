#include "test.h"

void ejson_obj_add_test()
{
    fprintf(stderr, "\n-- ejson_obj_add_test 1: add_eval() --\n");
    ejson e = ejso_new(_OBJ_);
    cstr k, ks, s; double d; int i;
    fprintf(stderr, "e: %s \n", s = ejso_toUStr(e)); ejss_free(s);
    ejso_addE(e, 0, s = "\"null\":null");              fprintf(stderr, "add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"false\":false");            fprintf(stderr, "add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"true\":true");              fprintf(stderr, "add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"int\": 100");               fprintf(stderr, "add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"double\":100.132");         fprintf(stderr, "add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"str\":\"this is a str\"");  fprintf(stderr, "add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"arr\": []");                fprintf(stderr, "add eval to e: %s \n", s);
    ejso_addE(e, 0, s = "\"obj\": {}");                fprintf(stderr, "add eval to e: %s \n", s);
    fprintf(stderr, "e to pretty str:\n%s\n", s = ejso_toFStr(e)); ejss_free(s);ejso_free(e);
    
    fprintf(stderr, "\n-- ejson_obj_add_test 2: add_*() --\n");
    e = ejso_new(_OBJ_);
    fprintf(stderr, "e: %s \n", s = ejso_toUStr(e)); ejss_free(s);
    ejso_addT(e, k = "key1", _FALSE_);                  fprintf(stderr, "add false  to e: k:%s val:none \n", k);
    ejso_addT(e, k = "key2", _TRUE_);                   fprintf(stderr, "add true   to e: k:%s val:none \n", k);
    ejso_addT(e, k = "key3", _NULL_);                   fprintf(stderr, "add null   to e: k:%s val:none \n", k);
    ejso_addF(e, k = "key4", i = 100);                  fprintf(stderr, "add number to e: k:%s val:%d   \n", k, i);
    ejso_addF(e, k = "key5", d = 100.1214);             fprintf(stderr, "add number to e: k:%s val:%f   \n", k, d);
    ejso_addS(e, k = "key6", s = "this is a str");      fprintf(stderr, "add str    to e: k:%s val:%s   \n", k, s);
    ejso_addT(e, k = "key7", _ARR_);                    fprintf(stderr, "add arrar  to e: k:%s val:none   \n", k);
    ejso_addT(e, k = "key8", _OBJ_);                    fprintf(stderr, "add obj    to e: k:%s val:none   \n", k);
    fprintf(stderr, "to pretty str:\n%s\n", s = ejso_toFStr(e)); ejss_free(s);ejso_free(e);
    
    
    fprintf(stderr, "\n-- ejson_obj_add_test 3: add_evalTo() --\n");
    e = ejss_eval("{}");
    ejso_addO(e, 0, ejss_eval("\"obj\":{}"));
    fprintf(stderr, "e: %s \n", s = ejso_toUStr(e)); ejss_free(s);
    ks = "obj";
    ejsk_addE(e, ks, 0, s = "\"null\":null");             fprintf(stderr, "add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"false\":false");           fprintf(stderr, "add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"true\":true");             fprintf(stderr, "add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"int\": 100");              fprintf(stderr, "add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"double\":100.132");        fprintf(stderr, "add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"str\":\"this is a str\""); fprintf(stderr, "add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"arr\": []");               fprintf(stderr, "add eval to e.%s: %s \n", ks, s);
    ejsk_addE(e, ks, 0, s = "\"obj\": {}");               fprintf(stderr, "add eval to e.%s: %s \n", ks, s);
    fprintf(stderr, "to pretty str:\n%s\n", s = ejso_toFStr(e)); ejss_free(s);ejso_free(e);
    
    fprintf(stderr, "\n-- ejson_obj_add_test 4: add_*To() --\n"); 
    e = ejss_eval("{}");
    ejso_addO(e, 0, ejss_eval("\"obj\":{}"));
    ejsk_addO(e, "obj", 0, ejss_eval("\"obj\":{}"));
    fprintf(stderr, "e: %s \n", s = ejso_toUStr(e)); ejss_free(s);
    ks = "obj";
    ejsk_addT(e, ks, k = "key1", _FALSE_);               fprintf(stderr, "add false  to e.%s: k:%s val:none \n", ks, k);
    ejsk_addT(e, ks, k = "key2", _TRUE_);                fprintf(stderr, "add true   to e.%s: k:%s val:none \n", ks, k);
    ejsk_addT(e, ks, k = "key3", _NULL_);                fprintf(stderr, "add null   to e.%s: k:%s val:none \n", ks, k);
    ejsk_addF(e, ks, k = "key4", i = 100);               fprintf(stderr, "add number to e.%s: k:%s val:%d   \n", ks, k, i);
    ejsk_addF(e, ks, k = "key5", d = 100.1214);          fprintf(stderr, "add number to e.%s: k:%s val:%f   \n", ks, k, d);
    ejsk_addS(e, ks, k = "key6", s = "this is a str");   fprintf(stderr, "add str    to e.%s: k:%s val:%s   \n", ks, k, s);
    ejsk_addT(e, ks, k = "key7", _ARR_);                 fprintf(stderr, "add arrar  to e.%s: k:%s val:none \n", ks, k);
    ejsk_addT(e, ks, k = "key8", _OBJ_);                 fprintf(stderr, "add obj    to e.%s: k:%s val:none \n", ks, k);
    fprintf(stderr, "to pretty str:\n%s\n", s = ejso_toFStr(e)); ejss_free(s);ejso_free(e);
    
    
    
}

