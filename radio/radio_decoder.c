/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-22     Rick       the first version
 */
#include <rtthread.h>
#include <stdio.h>
#include "radio_app.h"
#include "radio_decoder.h"
#include "board.h"
#include "Radio_Decoder.h"
#include "Radio_Encoder.h"
#include "Flashwork.h"
#include "led.h"
#include "key.h"
#include "pin_config.h"
#include "wifi-api.h"

#define DBG_TAG "RADIO_DECODER"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

void Device_Learn(Message_Format buf)
{
    if(Flash_Get_Key_Valid(buf.From_ID) != RT_EOK)
    {
        if(Get_MainNums()!=RT_EOK)
        {
            learn_fail();
            return;
        }
    }
    if(buf.From_ID>=10000000 && buf.From_ID<20000000)
    {
        switch(buf.Data)
        {
        case 1:
            SlaveDataEnqueue(buf.From_ID,buf.Counter,3,2);
            break;
        case 2:
            LOG_I("Learn Success\r\n");
            Del_Device(buf.From_ID);
            Device_Add2Flash_Wifi(buf.From_ID,0);
            learn_success();
        }
    }
}
void NormalSolve(int rssi,uint8_t *rx_buffer,uint8_t rx_len)
{
    Message_Format Rx_message;
    if(rx_buffer[rx_len]==0x0A&&rx_buffer[rx_len-1]==0x0D)
     {
         sscanf((const char *)&rx_buffer[1],"{%ld,%ld,%d,%d,%d}",&Rx_message.Target_ID,&Rx_message.From_ID,&Rx_message.Counter,&Rx_message.Command,&Rx_message.Data);
         if(Rx_message.Target_ID == Get_Self_ID())
         {
             if(Rx_message.From_ID == 98989898)
             {
                 LOG_I("Factory Test verify ok,RSSI is %d\r\n",rssi);
                 rf_refresh();
                 if(rssi>-70)
                 {
                     rf_led_factory(2);
                 }
                 else
                 {
                     rf_led_factory(1);
                }
                return;
             }
             switch(Rx_message.Command)
             {
             case 3://学习
                 Device_Learn(Rx_message);
                 break;
             }
             rf_led(3);
             Main_Heart(Rx_message.From_ID,1);
         }
     }
}
void GatewaySyncSolve(int rssi,uint8_t *rx_buffer,uint8_t rx_len)
{
    Message_Format Rx_message;
    if(rx_buffer[rx_len]=='A')
    {
        sscanf((const char *)&rx_buffer[2],"{%d,%d,%ld,%ld,%ld,%d,%d}",&Rx_message.ack,&Rx_message.type,&Rx_message.Target_ID,&Rx_message.From_ID,&Rx_message.Payload_ID,&Rx_message.Rssi,&Rx_message.Data);
        if(Rx_message.Target_ID == Get_Self_ID() && Flash_Get_Key_Valid(Rx_message.From_ID) == RT_EOK)
        {
            LOG_D("GatewaySyncSolve buf is %s,rssi is %d\r\n",rx_buffer,rssi);
            rf_led(3);
            if(Rx_message.ack)
            {
                GatewayDataEnqueue(Rx_message.From_ID,0,0,7,0);
            }
            Main_Rssi_Report(Rx_message.From_ID,rssi);
            Main_Heart(Rx_message.From_ID,1);
            switch(Rx_message.type)
            {
            case 1:
                Device_Heart(Rx_message.Payload_ID,1);//子设备心跳
                Slave_Rssi_Report(Rx_message.Payload_ID,Rx_message.Rssi);//rssi
                break;
            case 2:
                Local_Delete(Rx_message.Payload_ID);
                Del_Device(Rx_message.Payload_ID);//删除终端
                break;
            case 3://同步在线设备
                Sync_Refresh(Rx_message.From_ID);
                Device_Add2Flash_Wifi(Rx_message.Payload_ID,Rx_message.From_ID);//增加终端
                Device_Heart(Rx_message.Payload_ID,1);//子设备心跳
                Slave_Rssi_Report(Rx_message.Payload_ID,Rx_message.Rssi);//rssi
                WariningUpload(Rx_message.From_ID,Rx_message.Payload_ID,2,Rx_message.Data);//终端电量
                break;
            case 4://删除全部
                Del_MainBind(Rx_message.From_ID);
                break;
            case 5://同步离线设备
                Sync_Refresh(Rx_message.From_ID);
                Device_Add2Flash_Wifi(Rx_message.Payload_ID,Rx_message.From_ID);//增加终端
                Device_Heart(Rx_message.Payload_ID,0);//子设备心跳
                WariningUpload(Rx_message.From_ID,Rx_message.Payload_ID,2,Rx_message.Data);//终端低电量
                break;
            case 6://添加设备
                Device_Add2Flash_Wifi(Rx_message.Payload_ID,Rx_message.From_ID);//增加终端
                Device_Heart(Rx_message.Payload_ID,1);
                Slave_Rssi_Report(Rx_message.Payload_ID,Rx_message.Rssi);//心跳
                WariningUpload(Rx_message.From_ID,Rx_message.Payload_ID,2,Rx_message.Data);//终端低电量
                break;
            }
        }
    }
}
void GatewayWarningSolve(int rssi,uint8_t *rx_buffer,uint8_t rx_len)
{
    Message_Format Rx_message;
    if(rx_buffer[rx_len]=='B')
    {
        sscanf((const char *)&rx_buffer[2],"{%d,%ld,%ld,%ld,%d,%d,%d}",&Rx_message.ack,&Rx_message.Target_ID,&Rx_message.From_ID,&Rx_message.Payload_ID,&Rx_message.Rssi,&Rx_message.Command,&Rx_message.Data);
        if(Rx_message.Target_ID == Get_Self_ID() && Flash_Get_Key_Valid(Rx_message.From_ID) == RT_EOK)
        {
            LOG_D("GatewayWarningSolve buf %s,rssi is %d\r\n",rx_buffer,rssi);
            rf_led(3);
            if(Rx_message.ack)
            {
                GatewayDataEnqueue(Rx_message.From_ID,0,0,7,0);
            }
            Main_Rssi_Report(Rx_message.From_ID,rssi);
            Main_Heart(Rx_message.From_ID,1);
            LOG_D("WariningUpload From ID is %ld,Device ID is %ld,type is %d,value is %d\r\n",Rx_message.From_ID,Rx_message.Payload_ID,Rx_message.Command,Rx_message.Data);
            switch(Rx_message.Command)
            {
            case 1:
                WariningUpload(Rx_message.From_ID,Rx_message.Payload_ID,1,Rx_message.Data);//主控水警
                break;
            case 2:
                WariningUpload(Rx_message.From_ID,Rx_message.Payload_ID,0,Rx_message.Data);//主控阀门
                break;
            case 3:
                WariningUpload(Rx_message.From_ID,Rx_message.Payload_ID,2,Rx_message.Data);//主控测水线掉落
                break;
            case 4:
                Device_Heart(Rx_message.Payload_ID,0);//子设备离线
                break;
            case 5:
                Device_Heart(Rx_message.Payload_ID,1);//子设备心跳
                Slave_Rssi_Report(Rx_message.Payload_ID,Rx_message.Rssi);//rssi
                WariningUpload(Rx_message.From_ID,Rx_message.Payload_ID,1,Rx_message.Data);//终端水警
                MotoStateUpload(Rx_message.From_ID,0);//主控开关阀
                break;
            case 6:
                Device_Heart(Rx_message.Payload_ID,1);//子设备心跳
                Slave_Rssi_Report(Rx_message.Payload_ID,Rx_message.Rssi);//rssi
                WariningUpload(Rx_message.From_ID,Rx_message.Payload_ID,2,Rx_message.Data);//终端低电量
                if(Rx_message.Data==2)//ultra low
                {
                    MotoStateUpload(Rx_message.From_ID,0);//主控开关阀
                }
                break;
            case 7:
                InitWarn_Main(Rx_message.From_ID);//报警状态
                break;
            case 8://NTC报警
                WariningUpload(Rx_message.From_ID,Rx_message.Payload_ID,3,Rx_message.Data);
                break;
            case 9://终端掉落
                WariningUpload(Rx_message.From_ID,Rx_message.Payload_ID,3,Rx_message.Data);
                break;
            }
        }
    }
}
void GatewayControlSolve(int rssi,uint8_t *rx_buffer,uint8_t rx_len)
{
    Message_Format Rx_message;
    if(rx_buffer[rx_len]=='C')
    {
        sscanf((const char *)&rx_buffer[2],"{%d,%ld,%ld,%ld,%d,%d,%d}",&Rx_message.ack,&Rx_message.Target_ID,&Rx_message.From_ID,&Rx_message.Payload_ID,&Rx_message.Rssi,&Rx_message.Command,&Rx_message.Data);
        if(Rx_message.Target_ID == Get_Self_ID() && Flash_Get_Key_Valid(Rx_message.From_ID) == RT_EOK)
        {
            LOG_D("GatewayControlSolve buf %s,rssi is %d\r\n",rx_buffer,rssi);
            rf_led(3);
            if(Rx_message.ack)
            {
                GatewayDataEnqueue(Rx_message.From_ID,0,0,7,0);
            }
            Main_Rssi_Report(Rx_message.From_ID,rssi);
            Main_Heart(Rx_message.From_ID,1);
            switch(Rx_message.Command)
            {
            case 1:
                MotoStateUpload(Rx_message.From_ID,Rx_message.Data);//主控开关阀
                if(Rx_message.Data == 0)
                {
                    CloseWarn_Main(Rx_message.From_ID);
                }
                break;
            case 2:
                Device_Heart(Rx_message.Payload_ID,1);//子设备心跳
                Slave_Rssi_Report(Rx_message.Payload_ID,Rx_message.Rssi);//rssi
                if(Rx_message.Data == 0 || Rx_message.Data == 1)
                {
                    MotoStateUpload(Rx_message.From_ID,Rx_message.Data);//主控开关阀
                    if(Rx_message.Data == 0)
                    {
                        CloseWarn_Slave(Rx_message.Payload_ID);
                    }
                }
                else
                {
                    MotoStateUpload(Rx_message.From_ID,0);//主控开关阀
                }
                break;
            case 3:
                if(Rx_message.Payload_ID)//Delay远程关闭
                {
                    Door_Delay_WiFi(Rx_message.From_ID,Rx_message.Payload_ID,Rx_message.Data);
                    Device_Heart(Rx_message.Payload_ID,1);//子设备心跳
                    Slave_Rssi_Report(Rx_message.Payload_ID,Rx_message.Rssi);//rssi
                }
                break;
            case 4:
                MotoStateUpload(Rx_message.From_ID,Rx_message.Data);
                break;
            case 5:
                MotoStateUpload(Rx_message.From_ID,Rx_message.Data);//主控开关阀
                Ack_Report(Rx_message.From_ID);
                InitWarn_Main(Rx_message.From_ID);//报警状态
                break;
            case 6:
                DoorControlUpload(Rx_message.Payload_ID,Rx_message.Data);//主控开关阀
                Device_Heart(Rx_message.Payload_ID,1);//子设备心跳
                Slave_Rssi_Report(Rx_message.Payload_ID,Rx_message.Rssi);//rssi
                MotoStateUpload(Rx_message.From_ID,Rx_message.Data);//主控开关阀
                if(Rx_message.Data == 0)
                {
                    CloseWarn_Slave(Rx_message.Payload_ID);
                }
                break;
            }
        }
    }
}
void Radio_Parse(int rssi,uint8_t* data,size_t len)
{
    switch(data[1])
    {
    case '{':
        NormalSolve(rssi,data,len-1);
        break;
    case 'A':
        GatewaySyncSolve(rssi,data,len-1);
        break;
    case 'B':
        GatewayWarningSolve(rssi,data,len-1);
        break;
    case 'C':
        GatewayControlSolve(rssi,data,len-1);
        break;
    default:
        break;
    }
}

