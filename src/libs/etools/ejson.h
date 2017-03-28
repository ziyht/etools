/// =====================================================================================
///
///       Filename:  ejson.h
///
///    Description:  a easier way to handle json, you can also using it as a simple dic
///
///        Version:  0.8
///        Created:  12/18/2016 08:51:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __EJSON_H__
#define __EJSON_H__

#define EJSON_VERSION "ejson 0.8.3"      // add macro ejso_itrl

#include "etype.h"

/// -- pre def ---
typedef struct ejson_s* ejson, ** ejson_p;
typedef uint type;

#define MAX_KEY_LEN         512     // max len of keys

/// -- ejson types --
enum{
    EJSON_FALSE = 0x00u ,
    EJSON_TRUE  ,
    EJSON_NULL  ,
    EJSON_NUM   ,
    EJSON_STR   ,
    EJSON_PTR   ,
    EJSON_RAW   ,
    EJSON_ARR   = 0x0Eu ,
    EJSON_OBJ   ,

// -- short name of [ejson types] used for ejs_new(), close it if it conflict with other micors
#if 1
    _FALSE_ = EJSON_FALSE ,
    _TRUE_  ,
    _NULL_  ,
    _NUM_   ,
    _STR_   ,
    _PTR_   ,
    _RAW_   ,
    _ARR_   = EJSON_ARR ,
    _OBJ_   ,
#endif
};

typedef enum {
    ALL_ON      =  0xFF,
    ENDCHK_ON   =  0x01,     // requre null-terminated check in parse end
    CMMT_ON     =  0x02,     // requre comment supported
    ENDCHK_OFF  = ~0x01,
    CMMT_OFF    = ~0x02,
    ALL_OFF     =  0x00,
}opts;

#ifdef __cplusplus
extern "C" {
#endif

/// -- ejson new object --
/// @param src     - json string to parse to ejson
/// @param err_pos - point to the err position if parse err
/// @param opt     - opitions of parsing
/// @param file    - file to parse to ejson
/// @return ejson  - [ejson object] if parse ok
///                  [NULL] if parse err
///         constr - [constr] point to err position if check err
///                  [NULL] if check ok
///
ejson  ejso_new(type type);           // create a ejson obj

ejson  ejss_eval(constr src);         // parse a str  to ejson obj, can have key before of a obj, not support comment in default
ejson  ejss_evalOpts(constr src, constr* err_pos, opts opt);

int    ejss_check(constr src);        // only check the str, not alloc any memory, return 1 if the str ok, else return 0, using ejse_pos() to get err position int src
int    ejss_checkOpts(constr src, constr* err_pos, opts opt);

ejson  ejsf_eval(constr file);        // parse a file to ejson obj, support comment like: //, /**/, # in default
ejson  ejsf_evalOpts(constr file, constr* err_pos, opts opt);

// todo : ejse_set API name is not good

void   ejse_set(opts opt);            // set comment support of eval APIs, effect on ejss_eval(), ejss_check(), ejs*_add*eval()
void   ejsf_set(opts opt);            // set comment support of eval APIs, effect on ejsf_eval()

// todo : ejse_str API name is not good

/// -- ejson error --
constr ejse_str();                   // get the err_str      last recured, is not thread safe
constr ejse_pos();                   // get the err_position last recured, is not thread safe

/// -- ejson obj add
/// @param root - ejson object to be added to
/// @param obj  - ejson object to add to, the obj must not in another ejson, or add operation will faild
/// @param str  - json str, can have a key before, like: "key":{}, but
/// @param key  - str to be set as key to the ejson object to be added
/// @param keys - str to locate the specific ejson object in root, like: "key1.key2[0].key3"
/// @param val  - value to be set to the ejson object to be added
/// @return ejson - [ejson object] which is added if add ok
///               - [NULL] if add faild, use ejson_err() to get err info
///         void* - [void*] the ptr point to new alloced space
///               - [NULL] if add faild
///
/// @note: 1. if root obj to add to is a ARR obj, the key from param or obj/src will never check, use the exist key in obj or src
///        2. if root obj to add to is a OBJ obj, must have a key from param or obj/src, if key is set in param, the key in obj/src will always be replaced by it
///

ejson  ejso_addO(ejson root, constr key, ejson  obj);               // add an exist obj to root
ejson  ejso_addE(ejson root, constr key, constr src);               // add an eval  obj to root
ejson  ejso_addT(ejson root, constr key, int   type);               // add an opts  obj to root, only support FLASE,TURE,NULL,ARR,OBJ
ejson  ejso_addF(ejson root, constr key, double val);               // add an NUM   obj to root
ejson  ejso_addS(ejson root, constr key, constr val);               // add a  STR   obj to root
ejson  ejso_addP(ejson root, constr key, void*  ptr);               // add a  PTR   obj to root, the ptr will output like NUM obj when use all ejs_toStr()
void*  ejso_addR(ejson root, constr key, int    len);               // add a  RAW   obj to root, alloc a new space(len) for data, this obj will output with str value "RAW" when use all ejs_toStr()

ejson  ejsk_addO(ejson root, constr keys, constr key, ejson  obj);  // add an exist obj to specific obj in root via keys
ejson  ejsk_addE(ejson root, constr keys, constr key, constr src);  // add an eval  obj to specific obj in root via keys
ejson  ejsk_addT(ejson root, constr keys, constr key, int   type);  // add an opts  obj to specific obj in root via keys, only support FLASE,TURE,NULL,ARR,OBJ
ejson  ejsk_addF(ejson root, constr keys, constr key, double val);  // add an NUM   obj to specific obj in root via keys
ejson  ejsk_addS(ejson root, constr keys, constr key, constr val);  // add a  STR   obj to specific obj in root via keys
ejson  ejsk_addP(ejson root, constr keys, constr key, void*  ptr);  // add a  PTR   obj to specific obj in root via keys, the ptr will output like NUM obj when use all ejs_toStr()
void*  ejsk_addR(ejson root, constr keys, constr key, int    len);  // add a  RAW   obj to root, alloc a new space(len) for data, this obj will output with str value "RAW" when use all ejs_toStr()

ejson  ejsr_addO(ejson root, constr rawk, constr key, ejson  obj);  // add an exist obj to specific obj in root via keys
ejson  ejsr_addE(ejson root, constr rawk, constr key, constr src);  // add an eval  obj to specific obj in root via keys
ejson  ejsr_addT(ejson root, constr rawk, constr key, int   type);  // add an opts  obj to specific obj in root via keys, only support FLASE,TURE,NULL,ARR,OBJ
ejson  ejsr_addF(ejson root, constr rawk, constr key, double val);  // add an NUM   obj to specific obj in root via keys
ejson  ejsr_addS(ejson root, constr rawk, constr key, constr val);  // add a  STR   obj to specific obj in root via keys
ejson  ejsr_addP(ejson root, constr rawk, constr key, void*  ptr);  // add a  PTR   obj to specific obj in root via keys, the ptr will output like NUM obj when use all ejs_toStr()
void*  ejsr_addR(ejson root, constr rawk, constr key, int    len);  // add a  RAW   obj to root, alloc a new space(len) for data, this obj will output with str value "RAW" when use all ejs_toStr()

/// -- ejson del and free  --
void   ejso_free (ejson obj );                  // release all the resources of obj, there is no check whether the obj is in another ejson object, be careful
void   ejsk_free (ejson root, constr keys);     // remove and release all the resources of specific obj in root via keys
void   ejsr_free (ejson root, constr rawk);     // remove and release all the resources of specific obj in root via raw key

void   ejso_freeO(ejson root, ejson  obj );     // remove and release all the resources of specific obj in root
void   ejso_freeK(ejson root, constr keys);     // remove and release all the resources of specific obj in root via keys, same as ejsk_free()
void   ejso_freeR(ejson root, constr rawk);     // remove and release all the resources of specific obj in root via raw key, same as ejsr_free()

ejson  ejso_rmO(ejson root, ejson  obj );       // remove obj in root, return the del obj if del in root, else return NULL
ejson  ejso_rmK(ejson root, constr keys);       // remove the specific obj in root via keys   , return the del obj if del in root, else return NULL
ejson  ejso_rmR(ejson root, constr rawk);       // remove the specific obj in root via raw key, return the del obj if del in root, else return NULL

ejson  ejsk_rmO(ejson root, constr keys, ejson obj); // remove obj in the specific obj in root, return the del obj if del in specific obj, else return NULL
ejson  ejsr_rmO(ejson root, constr rawk, ejson obj); // remove obj in the specific obj in root, return the del obj if del in specific obj, else return NULL

ejson  ejso_pop(ejson root);                    // remove the first child in root                 and return it if root is ARR/OBJ type, else return NULL
ejson  ejsk_pop(ejson root, constr keys);       // remove the first child in specific obj in root and return it if root is ARR/OBJ type, else return NULL
ejson  ejsr_pop(ejson root, constr rawk);       // remove the first child in specific obj in root and return it if root is ARR/OBJ type, else return NULL

ejson  ejso_popT(ejson root);                   // remove the last  child in root                 and return it if root is ARR/OBJ type, else return NULL
ejson  ejsk_popT(ejson root, constr keys);      // remove the last  child in specific obj in root and return it if root is ARR/OBJ type, else return NULL
ejson  ejsr_popT(ejson root, constr rawk);      // remove the last  child in specific obj in root and return it if root is ARR/OBJ type, else return NULL

ejson  ejso_clear(ejson root);                  // remove and release all the children of obj, return obj itself if obj is OBJ or ARR, else return 0
ejson  ejsk_clear(ejson root, constr keys);     // remove and release all the children of specific obj in root via keys   , return specific obj if it is OBJ or ARR, else return 0
ejson  ejsr_clear(ejson root, constr rawk);     // remove and release all the children of specific obj in root via raw key, return specific obj if it is OBJ or ARR, else return 0

/// -- ejson obj wrap --
typedef struct ejsw_s* ejsw;

ejsw   ejsw_new   (uint  len);                          // get a ejson wraper, using wrapper you can warp ejson obj in resusing one buf

cstr   ejso_toFStr(ejson obj);                          // wrap the ejson obj to a pretty format string
cstr   ejso_toUStr(ejson obj);                          // wrap the ejson obj to a unformated string, take less space
cstr   ejsk_toFStr(ejson root, constr keys);            // wrap the specific ejson object in root to a pretty format string
cstr   ejsk_toUStr(ejson root, constr keys);            // wrap the specific ejson object in root to unformated string
cstr   ejsr_toFStr(ejson root, constr rawk);            // wrap the specific ejson object in root to a pretty format string
cstr   ejsr_toUStr(ejson root, constr rawk);            // wrap the specific ejson object in root to unformated string

cstr   ejso_toFWra(ejson obj , ejsw   w);               // wrap the ejson obj to a pretty format string
cstr   ejso_toUWra(ejson obj , ejsw   w);               // wrap the ejson obj to a unformated string, take less space
cstr   ejsk_toFWra(ejson root, constr keys, ejsw w);    // wrap the specific ejson object in root to a pretty format string
cstr   ejsk_toUWra(ejson root, constr keys, ejsw w);    // wrap the specific ejson object in root to unformated string
cstr   ejsr_toFWra(ejson root, constr rawk, ejsw w);    // wrap the specific ejson object in root to a pretty format string
cstr   ejsr_toUWra(ejson root, constr rawk, ejsw w);    // wrap the specific ejson object in root to unformated string

void   ejss_show  (constr s);                           // print the str  detail to stdout
void   ejsw_show  (ejsw   w);                           // print the ejsw detail to stdout

uint   ejss_len   (constr s);                           // only support the strs and constrs returned by ejson API
void   ejss_free  (cstr   s);                           // free the strs returned by ejson API, not do it on constr returned or other allocted buffers
cstr   ejsw_valS  (ejsw   w);                           // get the formated str val from wraper, do not free it
uint   ejsw_len   (ejsw   w);                           // get the formated str len from wraper
void   ejsw_free  (ejsw   w);                           // release the resource of wraper

/// -- ejson get --
constr ejso_keyS (ejson obj);                   // return "(nil)" if obj is NULL, else retuen key      of obj, do not free it
type   ejso_type (ejson obj);                   // return   -1    if obj is NULL, else return type     of obj, see ejson type macros
constr ejso_typeS(ejson obj);                   // return "(nil)" if obj is NULL, else return type_str of obj, do not free it, type_strs: "false" "true" "null" "number" "string" "array" "obj"
int    ejso_is   (ejson obj , int type);

ejson  ejsk      (ejson root, constr keys);     // return NULL if obj is NULL or not ARR/OBJ obj or keys is NULL or keys not found, else return the specific ejson obj
type   ejsk_type (ejson root, constr keys);     // return   -1    if obj is NULL or the specified obj is not exist, else return type     of obj, see ejson type macros
constr ejsk_typeS(ejson root, constr keys);     // return "(nil)" if obj is NULL or the specified obj is not exist, else return type_str of obj, do not free it, type_strs: "false" "true" "null" "number" "string" "array" "ejson object"
int    ejsk_is   (ejson root, constr keys, int type);

ejson  ejsr      (ejson root, constr rawk);     // return NULL if obj is NULL or not ARR/OBJ obj or keys is NULL or keys not found, else return the specific ejson obj
type   ejsr_type (ejson root, constr rawk);     // return   -1    if obj is NULL or the specified obj is not exist, else return type     of obj, see ejson type macros
constr ejsr_typeS(ejson root, constr rawk);     // return "(nil)" if obj is NULL or the specified obj is not exist, else return type_str of obj, do not free it, type_strs: "false" "true" "null" "number" "string" "array" "ejson object"
int    ejsr_is   (ejson root, constr rawk, int type);

/// -- ejson value --
int    ejso_valB(ejson obj);                    // return 0    if obj is NULL or the value is likely false, else return 1, not support ARR/OBJ obj(always return 0), and return 1 if str in STR obj is not empty
s64    ejso_valI(ejson obj);                    // return 0    if obj is NULL or not NUM     obj, else return int    value
double ejso_valF(ejson obj);                    // return 0.0  if obj is NULL or not NUM     obj, else return double value
constr ejso_valS(ejson obj);                    // return NULL if obj is NULL or not STR     obj, else return str    value
void*  ejso_valP(ejson obj);                    // return 0    if obj is NULL or not PTR     obj, else return ptr    value
void*  ejso_valR(ejson obj);                    // return NULL if obj is NULL or not RAW     obj, else return raw    ptr

int    ejsk_valB(ejson root, constr keys);      // return 0    if specific ejson obj is NULL or the value is likely false, else return 1, not support ARR/OBJ obj(always return 0), and always return 1 case STR obj
s64    ejsk_valI(ejson root, constr keys);      // return 0    if specific ejson obj is NULL or not NUM     obj, else return int    value of keys
double ejsk_valF(ejson root, constr keys);      // return 0.0  if specific ejson obj is NULL or not NUM     obj, else return double value of keys
constr ejsk_valS(ejson root, constr keys);      // return NULL if specific ejson obj is NULL or not STR     obj, else return str    value of keys
void*  ejsk_valP(ejson root, constr keys);      // return NULL if specific ejson obj is NULL or not PTR     obj or not add inside a ptr value, else return ptr    value of keys
void*  ejsk_valR(ejson root, constr keys);      // return NULL if specific ejson obj is NULL or not RAW     obj, else return raw    ptr   of keys

int    ejsr_valB(ejson root, constr rawk);      // return 0    if obj is NULL or the value is likely false, else return 1, not support ARR/OBJ obj(always return 0), and always return 1 case STR obj
s64    ejsr_valI(ejson root, constr rawk);      // return 0    if obj is NULL or not NUM     obj, else return int    value of raw key
double ejsr_valF(ejson root, constr rawk);      // return 0.0  if obj is NULL or not NUM     obj, else return double value of raw key
constr ejsr_valS(ejson root, constr rawk);      // return NULL if obj is NULL or not STR     obj, else return str    value of raw key
void*  ejsr_valP(ejson root, constr rawk);      // return 0    if obj is NULL or not PTR     obj, else return ptr    value of raw key
void*  ejsr_valR(ejson root, constr rawk);      // return NULL if obj is NULL or not RAW     obj, else return raw    ptr   of raw key

/// -- ejson len --
uint   ejso_len (ejson obj);                    // return the count of child in obj if it is ARR/OBJ obj, else return 0
uint   ejso_lenS(ejson obj);
uint   ejso_lenR(ejson obj);

uint   ejsk_len (ejson root, constr keys);      // return the count of child in the specified obj if it is "array" and "ejson object", else return 0
uint   ejsk_lenR(ejson root, constr keys);
uint   ejsk_lenS(ejson root, constr keys);

uint   ejsr_len (ejson root, constr rawk);      // return the count of child in the specified obj if it is "array" and "ejson object", else return 0
uint   ejsr_lenS(ejson root, constr rawk);
uint   ejsr_lenR(ejson root, constr rawk);

/// -- ejson iterating --
ejson  ejso_first(ejson obj);                   // return the first child of a obj if have, only support ARR/OBJ obj
ejson  ejso_last (ejson obj);                   // return the last  child of a obj if have, only support ARR/OBJ obj
ejson  ejso_next (ejson obj);                   // return the next  child of this obj in the same level if have, else return NULL
ejson  ejso_prev (ejson obj);                   // return the prev  child of this obj in the same level if have, else return NULL

ejson  ejsk_first(ejson obj, constr keys);      // return the first child of the specified obj if have, only support ARR/OBJ obj
ejson  ejsk_last (ejson obj, constr keys);      // return the last  child of the specified obj if have, only support ARR/OBJ obj

ejson  ejsr_first(ejson obj, constr rawk);      // return the first child of the specified obj if have, only support ARR/OBJ obj
ejson  ejsr_last (ejson obj, constr rawk);      // return the last  child of the specified obj if have, only support ARR/OBJ obj

#define ejso_itr(obj, itr)       for(ejson _INNER_ = ejso_first(obj)      ; (itr = _INNER_, _INNER_ = ejso_next(_INNER_), itr); )
#define ejsk_itr(obj, keys, itr) for(ejson _INNER_ = ejsk_first(obj, keys); (itr = _INNER_, _INNER_ = ejso_next(_INNER_), itr); )
#define ejsr_itr(obj, rawk, itr) for(ejson _INNER_ = ejsr_first(obj, rawk); (itr = _INNER_, _INNER_ = ejso_next(_INNER_), itr); )

#define ejso_itrl(obj, layer)       for(ejson _INNER_ = ejso_first(obj)      , itr##layer = 0; (itr##layer = _INNER_, _INNER_ = ejso_next(_INNER_), itr##layer); )
#define ejsk_itrl(obj, keys, layer) for(ejson _INNER_ = ejsk_first(obj, keys), itr##layer = 0; (itr##layer = _INNER_, _INNER_ = ejso_next(_INNER_), itr##layer); )
#define ejsr_itrl(obj, rawk, layer) for(ejson _INNER_ = ejsr_first(obj, rawk), itr##layer = 0; (itr##layer = _INNER_, _INNER_ = ejso_next(_INNER_), itr##layer); )

/// -- ejson compare --
int    ejso_cmpi(ejson obj, int    val);       // return -2 if obj is NULL, return -3 if type not match, return -1 if obj.i < val, return 1 if obj.i > val, else return 0
int    ejso_cmpf(ejson obj, double val);       // return -2 if obj is NULL, return -3 if type not match, return -1 if obj.i < val, return 1 if obj.i > val, else return 0
int    ejso_cmps(ejson obj, cstr   val);       // return -2 if obj is NULL, return -3 if type not match, return -4 if val is NULL, else return strcmp(obj.s, val)

int    ejsk_cmpi(ejson obj, constr keys, int    val);   // compare specified obj, return value is the same as ejson_cmpi()
int    ejsk_cmpf(ejson obj, constr keys, double val);   // compare specified obj, return value is the same as ejson_cmpf()
int    ejsk_cmps(ejson obj, constr keys, cstr   val);   // compare specified obj, return value is the same as ejson_cmps()

/// -- ejson set key --
///
/// @return - ejson: [NULL] set failed
///                  [obj] set ok
///

ejson ejso_setk(ejson obj, constr key);

/// -- ejson set val --
///
/// @return - ejson: [NULL] set failed
///                  [obj] set ok
///
ejson ejso_setT(ejson obj, uint  type);     // only support FALSE/TRUE/NULL obj
ejson ejso_setF(ejson obj, double val);     // only support NUM obj
ejson ejso_setS(ejson obj, constr val);     // only support STR obj
void* ejso_setR(ejson obj, uint   len);     // only support RAW obj
ejson ejso_setP(ejson obj, void*  ptr);     // only supprrt PTR obj

ejson ejsk_setT(ejson root, constr keys, uint  type);     // only support FALSE/TRUE/NULL obj
ejson ejsk_setF(ejson root, constr keys, double val);     // only support NUM obj
ejson ejsk_setS(ejson root, constr keys, constr val);     // only support STR obj
void* ejsk_setR(ejson root, constr keys, uint   len);     // only support RAW obj
ejson ejsk_setP(ejson root, constr keys, void*  ptr);     // only supprrt PTR obj

ejson ejsr_setT(ejson root, constr rawk, uint  type);     // only support FALSE/TRUE/NULL obj
ejson ejsr_setF(ejson root, constr rawk, double val);     // only support NUM obj
ejson ejsr_setS(ejson root, constr rawk, constr val);     // only support STR obj
void* ejsr_setR(ejson root, constr rawk, uint   len);     // only support RAW obj
ejson ejsr_setP(ejson root, constr rawk, void*  ptr);     // only supprrt PTR obj

/// -- ejson substitute string --
///
/// @return - ejson: [NULL] set failed
///                  [obj] set ok
///
ejson ejso_subk(ejson obj , constr subS, constr newS);

ejson ejso_subS(ejson obj , constr subS, constr newS);
ejson ejsk_subS(ejson root, constr keys, constr subS, constr newS);
ejson ejsr_subS(ejson root, constr rawk, constr subS, constr newS);

/// -- ejson counter --
///
/// @note: 1. if NUM obj not exsit in root, will auto create one, use all keys as rawk
///        2. the val of NUM obj will always >= 0
///
ejson  ejso_cntpp(ejson obj);                   // only support NUM obj
ejson  ejso_cntmm(ejson obj);                   // only support NUM obj

ejson  ejsk_cntpp(ejson root, constr keys);     // only support NUM obj
ejson  ejsk_cntmm(ejson root, constr keys);     // only support NUM obj

ejson  ejsr_cntpp(ejson root, constr rawk);     // only support NUM obj
ejson  ejsr_cntmm(ejson root, constr rawk);     // only support NUM obj

/// -- ejson sort --
typedef int (*__ecompar_fn) (ejson* _e1, ejson* _e2);
int    __KEYS_ACS(ejson* _e1, ejson* _e2);      // Ascending  via key string in all obj, dictionary sequence
int    __KEYS_DES(ejson* _e1, ejson* _e2);      // Descending via key string in all obj, dictionary sequence
int    __VALI_ACS(ejson* _e1, ejson* _e2);      // Ascending  via int value in NUM obj
int    __VALI_DES(ejson* _e1, ejson* _e2);      // Descending via int value in NUM obj

ejson  ejso_sort(ejson root, __ecompar_fn fn);
ejson  ejsk_sort(ejson root, constr keys, __ecompar_fn fn);
ejson  ejsr_sort(ejson root, constr rawk, __ecompar_fn fn);

/// -- todo: ejson compress and decompress
void*  ejso_lz4C(ejson obj);
ejson  ejso_lz4D(void* data);

/// -- ejson version
constr ejson_version();

#ifdef __cplusplus
}
#endif

#endif
