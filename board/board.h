/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for telakone-robokoura BOARD
 */

/*
 * Board identifier.
 */
#define BOARD_ROBOKOURA
#define BOARD_NAME                  "Telakone ROBOKOURA STM32F407ZG BOARD"

/*
 * Board oscillators-related settings.
 * NOTE: LSE not fitted.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                0U
#endif

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                25000000U
#endif

//#define STM32_HSE_BYPASS

/*
 * Board watchdog kick interval in milliseconds
 */
#if !defined(WDOG_KICK_INTERVAL)
#define WDOG_KICK_INTERVAL         1240U
#endif

/*
 * Ethernet MAC PHY address
 */
#if !defined(BOARD_PHY_ADDRESS)
#define BOARD_PHY_ADDRESS          0x1d
#endif

/*
 * Ethernet MAC PHY device ID 88E6020 Revision B1
 */
#if !defined(BOARD_PHY_ID)
#define BOARD_PHY_ID               0x00000205
#endif

/*
 * PHY is interfaced with RMII
 */
#define BOARD_PHY_RMII

/*
 * Board voltages.
 * Required for performance limits calculation.
 */
#define STM32_VDD                   300U

/*
 * MCU type as defined in the ST header.
 */
#define STM32F407xx

#include "iomap.h"

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
