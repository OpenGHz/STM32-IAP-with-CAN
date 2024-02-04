#include "iap.h"

iapfun jump2app; 

//WriteAddr:Ӧ�ó������ʼ��ַ������Ϊĳ��������ʼ��ַ
//pBuffer:Ӧ�ó���CODE.
//NumToWrite:Ӧ�ó����С(�ֽ�).

void iap_write_appbin(u32 WriteAddr,u8 *pBuffer,u32 NumToWrite)
{
	u32 i = 0;
	u16 temp;

    if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
    {
        printf("��ַԽ��!\r\n");
        return;   
    }
    
    if(WriteAddr % STM_SECTOR_SIZE)
    {
        printf("��ַ��FLASH�����׵�ַ!\r\n");
        return;
    }

    /*��ʼд��*/
    FLASH_Unlock();
    while(1)
    {
        if(NumToWrite == 0)//��һ�������պ�����д��
        {
            break;
        }
        else if(NumToWrite < STM_SECTOR_SIZE)//ʣ��Ҫд������ݲ���һ������
        {
            FLASH_ErasePage(WriteAddr); //����������� 
            
            for(i = 0; i < NumToWrite; i += 2)
            {
                temp  = (u16)pBuffer[i + 1] << 8;
                temp |= (u16)pBuffer[i];
                FLASH_ProgramHalfWord(WriteAddr, temp);
                WriteAddr += 2;
            }
            break; //д��������˳�while
        }
        else
        {
            FLASH_ErasePage(WriteAddr); //�����������
            
            //д����������
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
    FLASH_Lock(); //����
}

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ
void iap_load_app(u32 appxaddr)
{
    unsigned char i = 0;
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�
	{ 
        /* �׵�ַ��MSP����ַ+4�Ǹ�λ�жϷ�������ַ */
		jump2app=(iapfun)*(vu32*)(appxaddr+4);
            
         /* �ر�ȫ���ж� */
        __set_PRIMASK(1); 
                 
        /* �رյδ�ʱ������λ��Ĭ��ֵ */
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL = 0;
        
        /* ��������ʱ�ӵ�Ĭ��״̬ */
        RCC_DeInit();
        
        /* �ر������жϣ���������жϹ����־ */  
        for (i = 0; i < 8; i++)
        {
            NVIC->ICER[i]=0xFFFFFFFF;
            NVIC->ICPR[i]=0xFFFFFFFF;
        }
        
        /* ʹ��ȫ���ж� */ 
        __set_PRIMASK(0);
        
        /* ��RTOS���̣�����������Ҫ������Ϊ��Ȩ��ģʽ��ʹ��MSPָ�� */
        __set_CONTROL(0);
        
        MSR_MSP(*(vu32*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		jump2app();									//��ת��APP.
        
        /* ��ת�ɹ��Ļ�������ִ�е�����û�������������Ӵ��� */
        while (1)
        {

        }
	}
	else printf("APPջ����ַ���Ϸ�!!\r\n");
}
