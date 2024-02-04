#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
#include "usart.h"

//�û������Լ�����Ҫ����
#define STM32_FLASH_SIZE 64 	 		//��ѡSTM32��FLASH������С(��λΪK)

#if STM32_FLASH_SIZE < 256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else
#define STM_SECTOR_SIZE 2048
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH��ʼ��ַ
#define STM32_FLASH_BASE    0x08000000   	//STM32 FLASH����ʼ��ַ
#define FLASH_APP1_ADDR		0x08004000  	//Ӧ�ó�����ʼ��ַ(�����FLASH)
											//����0X08000000~0X08003FFF�Ŀռ�ΪBootloaderʹ�ã�16KB��	
                                            
typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.   
   
void iap_load_app(u32 appxaddr);			//��ת��APP����ִ��
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//��ָ����ַ��ʼ,д��bin


#endif







































