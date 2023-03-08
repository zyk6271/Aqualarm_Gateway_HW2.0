/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-27     Rick       the first version
 */
#ifndef APPLICATIONS_HEART_H_
#define APPLICATIONS_HEART_H_

#include "stdint.h"

void Remote_Device_Add(uint32_t device_id);
void Heart_Init(void);
uint8_t Device_Heart(uint32_t Device_ID,uint8_t Heart);

#endif /* APPLICATIONS_HEART_H_ */
