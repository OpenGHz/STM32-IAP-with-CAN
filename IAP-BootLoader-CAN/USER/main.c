#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "iap.h"
#include "can.h"
#include "timer.h"
#include "led.h"

/************************************************
 ALIENTEK战舰STM32开发板实验47
 IAP实验 Bootloader V1.0 代码
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司
 作者：正点原子 @ALIENTEK
************************************************/
#define WaitTimeMs 1500 //等待时间，ms，boot程序启动后未在该时间内发起通讯自动跳转进入app程序
extern int TickMs;

extern CanRxMsg RxMessage;
extern u8 CANRecFlag;

#define APP_LEN_MAX 10*1024
u8 APP_RX_BUF[APP_LEN_MAX] = {0};  //接收缓冲,最大10kb.
uint32_t APPLength = 0;
RCC_ClocksTypeDef RCC_CLK;
int main(void)
{
    int i = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    uart_init(9600);	//串口初始化为115200
    delay_init();	   	//延时初始化
    CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,2,CAN_Mode_Normal);//CAN初始化正常模式,波特率1000Kbps

    TIM3_Int_Init(9,7199);//5Khz的计数频率，计数到10为2ms
		RCC_GetClocksFreq(&RCC_CLK);//Get chip frequenciesLED_Init();
		LED_Init();
    while(1)
    {
        if(TickMs > WaitTimeMs)
        {
            printf("开始执行FLASH用户代码!!\r\n");
						LED2=!LED2;
            if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
            {
                iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
            }
            else
            {
                printf("非FLASH应用程序,无法执行!\r\n");
                TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
                TIM_Cmd(TIM3, DISABLE);
            }
						printf("等待升级!!\r\n");
            TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
            TIM_Cmd(TIM3, DISABLE);
						break;
        }
        if(CANRecFlag)
        {
            CANRecFlag = 0;
						LED2=!LED2;
            printf("等待升级!!\r\n");
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
            
            if(RxMessage.StdId == 0x01)//接收到bin数据包
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
                    printf("缓冲区容量不足!!\r\n");
								}
            }

            if(RxMessage.StdId == 0x02)
            {
                if(APPLength)
                {
                    printf("开始更新固件...\r\n");
										LED2=!LED2;
                    if(((*(vu32*)(APP_RX_BUF+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
                    {
                        iap_write_appbin(FLASH_APP1_ADDR,APP_RX_BUF,APPLength);//更新FLASH代码
                        printf("固件更新完成!\r\n");
                    }
                    else
                    {
												LED3=!LED3;
                        printf("非FLASH应用程序!\r\n");
                    }
                }
                else
                {
										LED3=!LED3;
                    printf("没有可以更新的固件!\r\n");
                }
            }

            if(RxMessage.StdId == 0x03)
            {
                printf("开始执行FLASH用户代码!!\r\n");
								LED2=!LED2;
                if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
                {
                    iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
                }
                else
                {
										LED3=!LED3;
                    printf("非FLASH应用程序,无法执行!\r\n");
                }
            }
        }
    }
}
