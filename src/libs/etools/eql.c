/// =====================================================================================
///
///       Filename:  eql.c
///
///    Description:  easy quicklist
///
///        Version:  1.0
///        Created:  08/01/2018 08:51:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================


#include "eql.h"


/**

  basic list:

  |cap|len|data  ... ... |end|
    2   2


  item: (dynamic ?? )

             addr out
              |
              |
              |
  |ctype|otype|len|cap|data |0
   :4     :4    4   4   cap  1
               -------
                  | - maybe not needed in some otype


  false: |ctype|otype|
  true : |ctype|otype|
  null : |ctype|otype|
  num  : |ctype|otype|8|
  ptr  : |ctype|otype|8|
  str  : |ctype|otype|4||4|1|...|1
  raw  : |ctype|otype|4||4|1|...|1
  obj  : |ctype|otype|...|


  */










