/**************************************************************************/
/*!
    @file     verify.h
    @author   hathach

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2018, hathach (tinyusb.org)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/
#ifndef TUSB_VERIFY_H_
#define TUSB_VERIFY_H_

#include <stdbool.h>
#include <stdint.h>
#include "tusb_option.h"
#include "tusb_compiler.h"

/*------------------------------------------------------------------*/
/* This file use an advanced macro technique to mimic the default parameter
 * as C++ for the sake of code simplicity. Beware of a headache macro
 * manipulation that you are told to stay away.
 *
 * e.g
 *
 * - VERIFY( cond ) will return false if cond is false
 * - VERIFY( cond, err) will return err instead if cond is false
 *------------------------------------------------------------------*/

#ifdef __cplusplus
 extern "C" {
#endif


//--------------------------------------------------------------------+
// VERIFY Helper
//--------------------------------------------------------------------+
#if CFG_TUSB_DEBUG >= 1
  #define _MESS_ERR(_err)         printf("%s: %d: failed, error = %s\n", __PRETTY_FUNCTION__, __LINE__, tusb_strerr[_err])
  #define _MESS_FAILED()          printf("%s: %d: failed\n", __PRETTY_FUNCTION__, __LINE__)
#else
  #define _MESS_ERR(_err)
  #define _MESS_FAILED()
#endif

// Halt CPU (breakpoint) when hitting error, only apply for Cortex M3, M4, M7
#if defined(__ARM_ARCH_7M__) || defined (__ARM_ARCH_7EM__)

static inline void verify_breakpoint(void)
{
  // Cortex M CoreDebug->DHCSR
  volatile uint32_t* ARM_CM_DHCSR =  ((volatile uint32_t*) 0xE000EDF0UL);

  // Only halt mcu if debugger is attached
  if ( (*ARM_CM_DHCSR) & 1UL ) __asm("BKPT #0\n");
}

#else
#define verify_breakpoint()
#endif

/*------------------------------------------------------------------*/
/* Macro Generator
 *------------------------------------------------------------------*/

// Helper to implement optional parameter for VERIFY Macro family
#define GET_3RD_ARG(arg1, arg2, arg3, ...)        arg3
#define GET_4TH_ARG(arg1, arg2, arg3, arg4, ...)  arg4

/*------------- Generator for VERIFY and VERIFY_HDLR -------------*/
#define VERIFY_DEFINE(_cond, _handler, _ret)  do { if ( !(_cond) ) { _handler; return _ret;  } } while(0)

/*------------- Generator for VERIFY_ERR and VERIFY_ERR_HDLR -------------*/
#define VERIFY_ERR_DEF2(_error, _handler)  \
    do {                                              \
      uint32_t _err = (uint32_t)(_error);             \
      if ( 0 != _err ) { _MESS_ERR(_err); _handler; return _err; }\
    } while(0)

#define VERIFY_ERR_DEF3(_error, _handler, _ret)  \
    do {                                              \
      uint32_t _err = (uint32_t)(_error);             \
      if ( 0 != _err ) { _MESS_ERR(_err); _handler; return _ret; }\
    } while(0)




/*------------------------------------------------------------------*/
/* VERIFY
 * - VERIFY_1ARGS : return false if failed
 * - VERIFY_2ARGS : return provided value if failed
 *------------------------------------------------------------------*/
#define VERIFY_1ARGS(_cond)                         VERIFY_DEFINE(_cond, , false)
#define VERIFY_2ARGS(_cond, _ret)                   VERIFY_DEFINE(_cond, , _ret)

#define VERIFY(...)                   GET_3RD_ARG(__VA_ARGS__, VERIFY_2ARGS, VERIFY_1ARGS)(__VA_ARGS__)


/*------------------------------------------------------------------*/
/* VERIFY WITH HANDLER
 * - VERIFY_HDLR_2ARGS : execute handler, return false if failed
 * - VERIFY_HDLR_3ARGS : execute handler, return provided error if failed
 *------------------------------------------------------------------*/
#define VERIFY_HDLR_2ARGS(_cond, _handler)           VERIFY_DEFINE(_cond, _handler, false)
#define VERIFY_HDLR_3ARGS(_cond, _handler, _ret)     VERIFY_DEFINE(_cond, _handler, _ret)

#define VERIFY_HDLR(...)              GET_4TH_ARG(__VA_ARGS__, VERIFY_HDLR_3ARGS, VERIFY_HDLR_2ARGS)(__VA_ARGS__)


/*------------------------------------------------------------------*/
/* VERIFY STATUS
 * - VERIFY_ERR_1ARGS : return status of condition if failed
 * - VERIFY_ERR_2ARGS : return provided status code if failed
 *------------------------------------------------------------------*/
#define VERIFY_ERR_1ARGS(_error)                       VERIFY_ERR_DEF2(_error, )
#define VERIFY_ERR_2ARGS(_error, _ret)                 VERIFY_ERR_DEF3(_error, ,_ret)

#define VERIFY_ERR(...)            GET_3RD_ARG(__VA_ARGS__, VERIFY_ERR_2ARGS, VERIFY_ERR_1ARGS)(__VA_ARGS__)

/*------------------------------------------------------------------*/
/* VERIFY STATUS WITH HANDLER
 * - VERIFY_ERR_HDLR_2ARGS : execute handler, return status if failed
 * - VERIFY_ERR_HDLR_3ARGS : execute handler, return provided error if failed
 *------------------------------------------------------------------*/
#define VERIFY_ERR_HDLR_2ARGS(_error, _handler)        VERIFY_ERR_DEF2(_error, _handler)
#define VERIFY_ERR_HDLR_3ARGS(_error, _handler, _ret)  VERIFY_ERR_DEF3(_error, _handler, _ret)

#define VERIFY_ERR_HDLR(...)       GET_4TH_ARG(__VA_ARGS__, VERIFY_ERR_HDLR_3ARGS, VERIFY_ERR_HDLR_2ARGS)(__VA_ARGS__)



/*------------------------------------------------------------------*/
/* ASSERT
 * basically VERIFY with verify_breakpoint() as handler
 * - 1 arg : return false if failed
 * - 2 arg : return error if failed
 *------------------------------------------------------------------*/
#define ASSERT_1ARGS(_cond)            VERIFY_DEFINE(_cond, _MESS_FAILED(); verify_breakpoint(), false)
#define ASSERT_2ARGS(_cond, _ret)      VERIFY_DEFINE(_cond, _MESS_FAILED(); verify_breakpoint(), _ret)

#define TU_ASSERT(...)             GET_3RD_ARG(__VA_ARGS__, ASSERT_2ARGS, ASSERT_1ARGS)(__VA_ARGS__)

/*------------------------------------------------------------------*/
/* ASSERT Error
 * basically VERIFY Error with verify_breakpoint() as handler
 *------------------------------------------------------------------*/
#define ASERT_ERR_1ARGS(_error)         VERIFY_ERR_DEF2(_error, verify_breakpoint())
#define ASERT_ERR_2ARGS(_error, _ret)   VERIFY_ERR_DEF3(_error, verify_breakpoint(), _ret)

#define TU_ASSERT_ERR(...)         GET_3RD_ARG(__VA_ARGS__, ASERT_ERR_2ARGS, ASERT_ERR_1ARGS)(__VA_ARGS__)

/*------------------------------------------------------------------*/
/* ASSERT HDLR
 *------------------------------------------------------------------*/

#ifdef __cplusplus
 }
#endif

#endif /* TUSB_VERIFY_H_ */