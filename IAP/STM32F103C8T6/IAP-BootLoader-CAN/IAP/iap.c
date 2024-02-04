#include "iap.h"

iapfun jump2app; 

//WriteAddr:应用程序的起始地址，必须为某扇区的起始地址
//pBuffer:应用程序CODE.
//NumToWrite:应用程序大小(字节).

void iap_write_appbin(u32 WriteAddr,u8 *pBuffer,u32 NumToWrite)
{
	u32 i = 0;
	u16 temp;

    if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
    {
        printf("地址越界!\r\n");
        return;   
    }
    
    if(WriteAddr % STM_SECTOR_SIZE)
    {
        printf("地址非FLASH扇区首地址!\r\n");
        return;
    }

    /*开始写入*/
    FLASH_Unlock();
    while(1)
    {
        if(NumToWrite == 0)//上一个扇区刚好完整写完
        {
            break;
        }
        else if(NumToWrite < STM_SECTOR_SIZE)//剩余要写入的内容不到一个扇区
        {
            FLASH_ErasePage(WriteAddr); //擦除这个扇区 
            
            for(i = 0; i < NumToWrite; i += 2)
            {
                temp  = (u16)pBuffer[i + 1] << 8;
                temp |= (u16)pBuffer[i];
                FLASH_ProgramHalfWord(WriteAddr, temp);
                WriteAddr += 2;
            }
            break; //写入结束，退出while
        }
        else
        {
            FLASH_ErasePage(WriteAddr); //擦除这个扇区
            
            //写入整个扇区
            for(i = 0; i < STM_SECTOR_SIZE; i += 2)
            {
                temp  = (u16)pBuffer[i + 1] << 8;
                temp |= (u16)pBuffer[i];
                FLASH_ProgramHalfWord(WriteAddr, temp);
                WriteAddr += 2;
            }
            pBuffer += STM_SECTOR_SIZE;
            NumToWrite -= STM_SECTOR_SIZE;
        }
    }
    FLASH_Lock(); //上锁
}

//跳转到应用程序段
//appxaddr:用户代码起始地址
void iap_load_app(u32 appxaddr)
{
    unsigned char i = 0;
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法
	{ 
        /* 首地址是MSP，地址+4是复位中断服务程序地址 */
		jump2app=(iapfun)*(vu32*)(appxaddr+4);
            
         /* 关闭全局中断 */
        __set_PRIMASK(1); 
                 
        /* 关闭滴答定时器，复位到默认值 */
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL = 0;
        
        /* 设置所有时钟到默认状态 */
        RCC_DeInit();
        
        /* 关闭所有中断，清除所有中断挂起标志 */  
        for (i = 0; i < 8; i++)
        {
            NVIC->ICER[i]=0xFFFFFFFF;
            NVIC->ICPR[i]=0xFFFFFFFF;
        }
        
        /* 使能全局中断 */ 
        __set_PRIMASK(0);
        
        /* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
        __set_CONTROL(0);
        
        MSR_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app();									//跳转到APP.
        
        /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
        while (1)
        {

        }
	}
	else printf("APP栈顶地址不合法!!\r\n");
}
