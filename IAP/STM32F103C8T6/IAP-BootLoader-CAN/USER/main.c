#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "iap.h"
#include "can.h"
#include "timer.h"
#include "led.h"

/************************************************
 ALIENTEKս��STM32������ʵ��47
 IAPʵ�� Bootloader V1.0 ����
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/
#define WaitTimeMs 1500 //�ȴ�ʱ�䣬ms��boot����������δ�ڸ�ʱ���ڷ���ͨѶ�Զ���ת����app����
extern int TickMs;

extern CanRxMsg RxMessage;
extern u8 CANRecFlag;

#define APP_LEN_MAX 10*1024
u8 APP_RX_BUF[APP_LEN_MAX] = {0};  //���ջ���,���10kb.
uint32_t APPLength = 0;
RCC_ClocksTypeDef RCC_CLK;
int main(void)
{
    int i = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(9600);	//���ڳ�ʼ��Ϊ115200
    delay_init();	   	//��ʱ��ʼ��
    CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,2,CAN_Mode_Normal);//CAN��ʼ������ģʽ,������1000Kbps

    TIM3_Int_Init(9,7199);//5Khz�ļ���Ƶ�ʣ�������10Ϊ2ms
		RCC_GetClocksFreq(&RCC_CLK);//Get chip frequenciesLED_Init();
		LED_Init();
    while(1)
    {
        if(TickMs > WaitTimeMs)
        {
            printf("��ʼִ��FLASH�û�����!!\r\n");
						LED2=!LED2;
            if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
            {
                iap_load_app(FLASH_APP1_ADDR);//ִ��FLASH APP����
            }
            else
            {
                printf("��FLASHӦ�ó���,�޷�ִ��!\r\n");
                TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
                TIM_Cmd(TIM3, DISABLE);
            }
						printf("�ȴ�����!!\r\n");
            TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
            TIM_Cmd(TIM3, DISABLE);
						break;
        }
        if(CANRecFlag)
        {
            CANRecFlag = 0;
						LED2=!LED2;
            printf("�ȴ�����!!\r\n");
            TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
            TIM_Cmd(TIM3, DISABLE);
            break;
        }
    }

    while(1)
    {
        if(CANRecFlag)
        {
            CANRecFlag = 0;
            
            if(RxMessage.StdId == 0x01)//���յ�bin���ݰ�
            {
                if(APPLength < APP_LEN_MAX)
                {
                    for(i = 0; i <RxMessage.DLC; i++)
                    {
                        APP_RX_BUF[APPLength] = RxMessage.Data[i];
                        APPLength++;
                    }
                }
                else
								{
									  LED3=!LED3;
                    printf("��������������!!\r\n");
								}
            }

            if(RxMessage.StdId == 0x02)
            {
                if(APPLength)
                {
                    printf("��ʼ���¹̼�...\r\n");
										LED2=!LED2;
                    if(((*(vu32*)(APP_RX_BUF+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
                    {
                        iap_write_appbin(FLASH_APP1_ADDR,APP_RX_BUF,APPLength);//����FLASH����
                        printf("�̼��������!\r\n");
                    }
                    else
                    {
												LED3=!LED3;
                        printf("��FLASHӦ�ó���!\r\n");
                    }
                }
                else
                {
										LED3=!LED3;
                    printf("û�п��Ը��µĹ̼�!\r\n");
                }
            }

            if(RxMessage.StdId == 0x03)
            {
                printf("��ʼִ��FLASH�û�����!!\r\n");
								LED2=!LED2;
                if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
                {
                    iap_load_app(FLASH_APP1_ADDR);//ִ��FLASH APP����
                }
                else
                {
										LED3=!LED3;
                    printf("��FLASHӦ�ó���,�޷�ִ��!\r\n");
                }
            }
        }
    }
}
