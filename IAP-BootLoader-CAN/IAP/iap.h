#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
#include "usart.h"

//用户根据自己的需要设置
#define STM32_FLASH_SIZE 64 	 		//所选STM32的FLASH容量大小(单位为K)

#if STM32_FLASH_SIZE < 256
#define STM_SECTOR_SIZE 1024 //字节
#else
#define STM_SECTOR_SIZE 2048
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH起始地址
#define STM32_FLASH_BASE    0x08000000   	//STM32 FLASH的起始地址
#define FLASH_APP1_ADDR		0x08004000  	//应用程序起始地址(存放在FLASH)
											//保留0X08000000~0X08003FFF的空间为Bootloader使用（16KB）	
                                            
typedef  void (*iapfun)(void);				//定义一个函数类型的参数.   
   
void iap_load_app(u32 appxaddr);			//跳转到APP程序执行
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//在指定地址开始,写入bin


#endif







































