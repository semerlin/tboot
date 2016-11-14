#ifndef _ECC_H_
  #define _ECC_H_

#include "stddef.h"



extern int32_t ecc_calculate(__in const uint8_t *data, __out uint8_t *ecc_code);
extern int32_t ecc_correct_data(__in uint8_t *data, __in uint8_t *read_ecc, __in uint8_t *calc_ecc);












#endif

