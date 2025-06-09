//*****************************************************************************
//
// Copyright (C) 2002 - 2013 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the
//  distribution.
//
//  Neither the name of Texas Instruments Incorporated nor the names of
//  its contributors may be used to endorse or promote products derived
//  from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//****************************************************************************

// Hardware Interface Layer (HIL).
//
// Version 1.2.6.0
//
// Texas Instruments, 2002

// The following functions must be supplied by the user in order for the MSP430 DLL to control the device.

#ifndef _HIL_H_
#define _HIL_H_

// #includes. -----------------------------------------------------------------

#include <windows.h>

// #defines. ------------------------------------------------------------------

#ifndef _BASIC_TYPES_H_

typedef unsigned char BYTE;
typedef unsigned short WORD;

typedef enum HIL430_RETURN_CODES
{
	/// API function was NOT executed sucessfully.
	STATUS_ERROR = -1,
	/// API function was executed sucessfully.
    STATUS_OK
}STATUS_T;

/*
typedef LONG STATUS_T;

// Status codes.
enum {
    STATUS_ERROR = -1,
    STATUS_OK,
};
*/
#define _BASIC_TYPES_H_
#endif

#define LOW        0
#define HIGH       1
#define DRIVE_HIGH 2

enum
{
    POS_EDGE = 2,
    NEG_EDGE,
};

enum {JTAG = 0, SPYBIWIRE, SPYBIWIREJTAG};

#ifdef __cplusplus
extern "C" {
#endif

// Functions. -----------------------------------------------------------------
//void WINAPI HIL_SwitchTo4WireJtag (void);
void HIL_sbw_ExecuteFuseBlow(void);
void HIL_sbw_StepPSA(LONG Length);
void WINAPI HIL_CheckJtagFuse(void);
void WINAPI HIL_ResetJtagTap(void);

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_SetProtokol(CHAR* port);

Description:
 Sets the Protocol type (actual: JTAG, SPYBYWIRE)

Parameters:
 protocol_id:  Protocol type (Default: JTAG)

Returns:
 STATUS_OK:    The interface was initialized.
 STATUS_ERROR: The interface was not initialized.

Notes:
*/
STATUS_T WINAPI HIL_SetProtocol(int protocol_id);

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_Initialize(CHAR* port);

Description:
 Initialize the interface.

Parameters:
 port:    Interface port reference (application specific).

Returns:
 STATUS_OK:    The interface was initialized.
 STATUS_ERROR: The interface was not initialized.

Notes:
 1. port is the parameter provided to MSP430_Initialize().
*/
STATUS_T WINAPI HIL_Initialize(CHAR* port);

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_Open(void);

Description:
 Enable the JTAG interface to the device.

Parameters:

Returns:
 STATUS_OK:    The JTAG interface was opened.
 STATUS_ERROR: The JTAG interface was not opened.

Notes:
 1. The setting of Vpp to 0 is dependent upon the interface hardware.
 2. HIL_Open() calls HIL_Connect().
*/
STATUS_T WINAPI HIL_Open(void);

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_Connect(void);

Description:
 Enable the JTAG connection to the device.

Parameters:

Returns:
 STATUS_OK:    The JTAG connection to the device was enabled.
 STATUS_ERROR: The JTAG connection to the device was not enabled.

Notes:
*/
STATUS_T WINAPI HIL_Connect(void);
void WINAPI HIL_HoldReset_EntrySequence(void);

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_Release(void);

Description:
 Release the JTAG interface to the device.

Parameters:

Returns:
 STATUS_OK:    The interface was released.
 STATUS_ERROR: The interface was not released.

Notes:
 1. All JTAG interface signals should be tristated and negated.
*/
STATUS_T WINAPI HIL_Release(void);

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_Close(LONG vccOff);

Description:
 Close the interface.

Parameters:
 vccOff: Turn off the device Vcc (0 volts) if TRUE.

Returns:
 STATUS_OK:    The interface was closed.
 STATUS_ERROR: The interface was not closed.

Notes:
*/
STATUS_T WINAPI HIL_Close(LONG vccOff);

/* ----------------------------------------------------------------------------
Function:
 LONG WINAPI HIL_JTAG_START_QUEUE(LONG lQueueSize);

Description:
 A Queue for sending JTAG instructions through the Driver will be
 setup.

Parameters:
 lQueueSize: Number of 8 bit instructions with a buffer size of 16bits
             16 instruction are counted twice

Returns:
 STATUS_OK:    The interface was closed.
 STATUS_ERROR: The interface was not closed.

Notes:
 1. It is not possilbe to use return values from the TDO when sending
    date in this buffer queue.
*/
LONG WINAPI HIL_JTAG_START_QUEUE( LONG lQueueSize);

/* ----------------------------------------------------------------------------
Function:
 LONG WINAPI HIL_JTAG_END_QUEUE(void);

Description:
 Sends the Queue which have be setup since starting it.

Parameters:
 None 

Returns:
 STATUS_OK:    The interface was closed.
 STATUS_ERROR: The interface was not closed.

Notes:
 1. It is not possilbe to use return values from the TDO when sending
    date in this buffer queue.
*/
LONG WINAPI HIL_JTAG_END_QUEUE(void);

/* ----------------------------------------------------------------------------
Function:
 LONG WINAPI HIL_JTAG_IR(LONG instruction);

Description:
 The specified JTAG instruction is shifted into the device.

Parameters:
 instruction: The JTAG instruction to be shifted into the device.

Returns:
 The byte shifted out from the device (on TDO).

Notes:
 1. The byte instruction is passed as a LONG
 2. The byte result is returned as a LONG.
 3. This function must operate in conjunction with HIL_TEST_VPP(). When the parameter to
    HIL_TEST_VPP is FALSE, shift instructions into the device via TDO. No results are shifted out.
    When the parameter to HIL_TEST_VPP is TRUE, shift instructions into the device via TDI/VPP and
    shift results out of the device via TDO. 
*/
LONG WINAPI HIL_JTAG_IR(LONG instruction);

/* ----------------------------------------------------------------------------
Function:
 LONG WINAPI HIL_TEST_VPP(LONG mode);

Description:
 Set the operational mode of HIL_JTAG_IR().

Parameters:
 mode: FALSE: JTAG instructions are shifted into the device via TDO. No results are shifted out.
              During secure operations, Vpp is applied on TDI/VPP.
       TRUE:  JTAG instructions are shifted into the device via TDI/VPP and results are shifted out
              via TDO. During secure operations, Vpp is applied on TEST.

Returns:
 The previous mode (FALSE: TDO, TRUE: TDI/VPP).

Notes:
 1. This function operates in conjunction with HIL_JTAG_IR() and HIL_VPP().
 2. Since the FET Interface Module does not support routing the shift-in bit stream to TDO, this
    function has no significant effect (other than setting the associated file global variable).
*/
LONG WINAPI HIL_TEST_VPP(LONG mode);

/* ----------------------------------------------------------------------------
Function:
 LONG WINAPI HIL_JTAG_DR(LONG data, LONG bits);

Description:
 The specified JTAG data is shifted into the device.

Parameters:
 data: The JTAG data to be shifted into the device.
 bits: The number of JTAG data bits to be shifted into the device (8 or 16).

Returns:
 "bits" bits shifted out from the device (on TDO).

Notes:
 1. The byte or word data is passed as a LONG.
 2. The byte or word result is returned as a LONG.
*/
LONG WINAPI HIL_JTAG_DR(LONG data, LONG bits);

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_VCC(LONG voltage);

Description:
 Set the device Vcc pin to voltage/1000 volts.

Parameters:
 voltage: The device Vcc pin is set to voltage/1000 volts.

Returns:
 STATUS_OK:    The Vcc was set to voltage.
 STATUS_ERROR: The Vcc was not set to voltage.

Notes:
 1. This function is dependant upon the interface hardware. The FET interface module does not
    support this functionality.
 2. A "voltage" of zero (0) turns off voltage to the device.
 3. If the interface hardware does not support setting the device voltage to a specific value,
    a non-zero value should cause the device voltage to be set to a value within the device
    Vcc specification (i.e., a default Vcc). Insure that the default Vcc voltage supports FLASH
    operations.
 4. Insure that Vcc is stable before returning from this function.
*/
STATUS_T WINAPI HIL_VCC(LONG voltage);

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_TST(LONG state);

Description:
 Set the state of the device TST pin.

Parameters:
 state: The device TST pin is set to state (0/1).

Returns:

Notes:
 1. Not all MSP430 devices have a TST pin.
*/
void WINAPI HIL_TST(LONG state);

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_TCK(LONG state);

Description:
 Set the state of the device TCK pin.

Parameters:
 state: The device TCK pin is set to state (0/1/POS_EDGE (0->1)/NEG_EDGE (1->0)).

Returns:

Notes:
*/
void WINAPI HIL_TCK(LONG state);

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_TMS(LONG state);

Description:
 Set the state of the device TMS pin.

Parameters:
 state: The device TMS pin is set to state (0/1).

Returns:

Notes:
*/
void WINAPI HIL_TMS(LONG state);

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_TDI(LONG state);

Description:
 Set the state of the device TDI pin.

Parameters:
 state: The device TDI pin is set to state (0/1).

Returns:

Notes:
*/
void WINAPI HIL_TDI(LONG state);

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_TCLK(LONG state);

Description:
 Set the state of the device TCLK pin.

Parameters:
 state: The device TCLK pin is set to state (0/1/POS_EDGE (0->1)/NEG_EDGE (1->0)).

Returns:

Notes:
 1. The TCLK "pin" is actually the TDI pin.
*/
void WINAPI HIL_TCLK(LONG state);

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_RST(LONG state);

Description:
 Set the state of the device RST pin.

Parameters:
 state: The device RST pin is set to state (0/1).

Returns:

Notes:
 1. As implemented, a state == 0 asserts RST (active low), while a state == 1 tri-states RST.
    Thus, a pull-up resistor on RST is assumed.
 2. As implemented, EN_TCLK and RST are negative logic.
*/
void WINAPI HIL_RST(LONG state);

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_VPP(LONG voltage);

Description:
 Set the device Vpp pin to voltage/1000 volts.

Parameters:
 voltage: The device Vpp pin is set to voltage/1000 volts.

Returns:
 STATUS_OK:    The Vpp was set to voltage.
 STATUS_ERROR: The Vpp was not set to voltage.

Notes:
 1. This function is dependant upon the interface hardware. The FET interface module does not
    support this functionality.
 2. A "voltage" of zero (0) turns off Vpp voltage to the device.
 3. If the interface hardware does not support setting the device Vpp voltage to a specific value,
    a non-zero value should cause the device Vpp voltage to be set to a value within the device
    Vpp specification (i.e., a default Vpp). Insure that the default Vpp voltage supports FLASH
    operations.
 4. The parameter to HIL_TEST_VPP() can be used to determine if VPP is applied to TDI/VPP (FALSE)
    or to TEST/VPP (TRUE).
 5. Insure that Vpp is stable before returning from this function.
*/
STATUS_T WINAPI HIL_VPP(LONG voltage);

// Time delay and timer functions ---------------------------------------------

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_DelayMSec(LONG mSeconds);

Description:
 Delay for mSeconds milliseconds.

Parameters:
 mSeconds: The delay time (milliseconds).

Returns:

Notes:
 1. The precision of this delay function does not have to be high; "approximate" milliseconds delay is
    sufficient. Rather, the length of the delay needs to be determined precisely. The length of the delay
    is determined precisely by computing the difference of a timer value read before the delay and the
    timer value read after the delay.
*/
void WINAPI HIL_DelayMSec(LONG mSeconds);
void WINAPI HIL_Delay2uSec(LONG uSeconds);

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_StartTimer(void);

Description:
 Start the (precision) timer.

Parameters:

Returns:

Notes:
 The timer should have a resolution of atleast one millisecond.
*/
void WINAPI HIL_StartTimer(void);

/* ----------------------------------------------------------------------------
Function:
 ULONG WINAPI HIL_ReadTimer(void);

Description:
 Read the (precision) timer.

Parameters:

Returns:
 The value of the timer.

Notes:
 The timer should have a resolution of atleast one millisecond.
*/
ULONG WINAPI HIL_ReadTimer(void);

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_StopTimer(void);

Description:
 Stop the (precision) timer.

Parameters:

Returns:

Notes:
*/
void WINAPI HIL_StopTimer(void);

/* ----------------------------------------------------------------------------
Function:
 LONG WINAPI HIL_TDO(void);

Description:
 Reads the state of the device TDO pin.

Parameters:
 None.

Returns:
 TDO pin state.

Notes:
*/
LONG WINAPI HIL_TDO(void);

// ----------------------------------------------------------------------------

STATUS_T WINAPI HIL_Trace(BOOL OnOff, char* str);
#define HIL_TraceOn(STR) HIL_Trace(TRUE, STR)
#define HIL_TraceOff(STR) HIL_Trace(FALSE, STR)
STATUS_T WINAPI HIL_EdtLog(char* str);

// ----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // _HIL_H_
