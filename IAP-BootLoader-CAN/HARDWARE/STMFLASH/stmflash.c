#include "stmflash.h"
#include "delay.h"
#include "usart.h"
 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//STM32 FLASH ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/13
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
 
//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	}  
}
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊĳһ��������ʼ��ַ!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊĳһ��������ʼ��ַ)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���)

void STMFLASH_Write(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)
{    
    if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
        return;                               //�Ƿ���ַ
      
    /*��ʼд��*/
    FLASH_Unlock(); 
    while(1)
    {
        if(NumToWrite == 0)//��һ�������պ�����д��
        {
            break;
        }
        else if(NumToWrite < (STM_SECTOR_SIZE / 2))//ʣ��Ҫд������ݲ���һ������
        {
            FLASH_ErasePage(WriteAddr); //�����������  
            STMFLASH_Write_NoCheck(WriteAddr, pBuffer, NumToWrite); 
            break; //д�����������while
        }
        else
        {
            FLASH_ErasePage(WriteAddr); //�����������
            STMFLASH_Write_NoCheck(WriteAddr, pBuffer, STM_SECTOR_SIZE / 2); //д����������
           
            WriteAddr += STM_SECTOR_SIZE;
            pBuffer += STM_SECTOR_SIZE / 2;
            NumToWrite -= STM_SECTOR_SIZE / 2;        
        }
    }
    FLASH_Lock(); //����
}
#endif

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//WriteAddr:��ʼ��ַ
//WriteData:Ҫд�������
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//д��һ���� 
}
















