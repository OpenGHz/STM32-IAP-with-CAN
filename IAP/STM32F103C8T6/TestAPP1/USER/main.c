#include "sys.h"
#include "can.h"

RCC_ClocksTypeDef RCC_CLK;
int main(void)
{
	SCB->VTOR = FLASH_BASE | 0x4000;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,2,CAN_Mode_Normal); //CAN初始化正常模式,波特率1000Kbps
	RCC_GetClocksFreq(&RCC_CLK); //Get chip frequencies
	while(1)
	{
			if(CANRecFlag)
			{
					CANRecFlag = 0;
					Can_Send_Msg(0x66, RxMessage.Data, RxMessage.DLC);
			}
	}
}
