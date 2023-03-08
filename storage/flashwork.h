#ifndef __FLASHWORK_H__
#define __FLASHWORK_H__

#include <stdint.h>

#define MaxSupport 50
#define MainSupport 3
typedef struct
{
    uint8_t Num;
    uint8_t MainNum;
    uint8_t Rssi[MaxSupport];
    uint8_t Moto[MaxSupport];
    uint8_t Heart[MaxSupport];
    uint8_t HeartRetry[MaxSupport];
    uint8_t HeartCount[MaxSupport];
    uint8_t HeartRecv[MaxSupport];
    uint8_t UploadFlag[MaxSupport];
    uint8_t SyncRetry[MaxSupport];
    uint8_t SyncRecv[MaxSupport];
    uint32_t ID[MaxSupport];
    uint32_t Bind_ID[MaxSupport];
}Device_Info;

typedef struct
{
    uint8_t Num;
    uint32_t ID[MaxSupport];
}Remote_Info;

int flash_Init(void);
uint8_t Del_MainBind(uint32_t Device_ID);
uint8_t Del_Device(uint32_t Device_ID);
void LoadDevice2Memory(void);
uint32_t Flash_Get_Key_Value(uint8_t type,uint32_t key);
uint8_t Flash_Get_Key_Valid(uint32_t Device_ID);
uint32_t GetDoorID(uint32_t Main_ID);
uint8_t MainAdd_Flash(uint32_t Device_ID);
void Flash_Heart_Change(uint32_t Device_ID,uint32_t value);
uint8_t SlaveAdd_Flash(uint32_t Device_ID,uint32_t Bind_ID);
uint8_t DoorAdd_Flash(uint32_t Device_ID,uint32_t Bind_ID);
uint32_t GetBindID(uint32_t Device_ID);
uint8_t Flash_Get_Heart(uint32_t Device_ID);
uint8_t Flash_Set_Heart(uint32_t Device_ID,uint8_t heart);
uint8_t Get_MainNums(void);
uint8_t Get_Main_Valid(uint32_t device_id);
uint8_t Flash_Get_UploadFlag(uint32_t Device_ID);
uint8_t Flash_Set_UploadFlag(uint32_t Device_ID,uint8_t Flag);
uint8_t Flash_Get_Moto(uint32_t Device_ID);
uint8_t Flash_Set_Moto(uint32_t Device_ID,uint8_t Flag);
uint8_t Flash_Get_Rssi(uint32_t Device_ID);
uint8_t Flash_Set_Rssi(uint32_t Device_ID,uint8_t Flag);

#endif

