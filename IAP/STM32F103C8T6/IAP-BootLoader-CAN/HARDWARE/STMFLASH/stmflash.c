#include "stmflash.h"
#include "delay.h"
#include "usart.h"
 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//STM32 FLASH 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/13
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
 
//读取指定地址的半字(16位数据)
//faddr:读地址(此地址必须为2的倍数!!)
//返回值:对应数据.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	//如果使能了写   
//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//地址增加2.
	}  
}
//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为某一扇区的起始地址!)
//pBuffer:数据指针
//NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//最多是2K字节
//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为某一扇区的起始地址)
//pBuffer:数据指针
//NumToWrite:半字(16位)数(就是要写入的16位数据的个数)

void STMFLASH_Write(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)
{    
    if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
        return;                               //非法地址
      
    /*开始写入*/
    FLASH_Unlock(); 
    while(1)
    {
        if(NumToWrite == 0)//上一个扇区刚好完整写完
        {
            break;
        }
        else if(NumToWrite < (STM_SECTOR_SIZE / 2))//剩余要写入的内容不到一个扇区
        {
            FLASH_ErasePage(WriteAddr); //擦除这个扇区  
            STMFLASH_Write_NoCheck(WriteAddr, pBuffer, NumToWrite); 
            break; //写入结束，结束while
        }
        else
        {
            FLASH_ErasePage(WriteAddr); //擦除这个扇区
            STMFLASH_Write_NoCheck(WriteAddr, pBuffer, STM_SECTOR_SIZE / 2); //写入整个扇区
           
            WriteAddr += STM_SECTOR_SIZE;
            pBuffer += STM_SECTOR_SIZE / 2;
            NumToWrite -= STM_SECTOR_SIZE / 2;        
        }
    }
    FLASH_Lock(); //上锁
}
#endif

//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr+=2;//偏移2个字节.	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//WriteAddr:起始地址
//WriteData:要写入的数据
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//写入一个字 
}
















