#include "stm32f4xx.h"
#include "base_timer.h"
#include "pwm.h"
#include "delay.h"
#include "led.h"
#include "uart.h"
#include "button.h"
#include "sonic.h"
#include "gtimer.h"
#include "dht11.h"
#include "iwdg.h"
#include "wwdg.h"
#include "rtc.h"
#include "flash.h"
#include "adc.h"
#include "hspi.h"
#include "w25qxx.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

My_Flash_Typedef My_Flash;
MyADC_Typedef ADC_Temp;
W25QXX_Typedef W25Q64;

#define countof(a)      (sizeof(a) / sizeof(*(a)))
#define BufferSize (countof(Tx_Buffer)-1)

uint8_t Tx_Buffer[] = "JackTJH@nuaa.edu.cn";
uint8_t Rx_Buffer[BufferSize];

#if defined(WWDG_ON	)
WWDG_Cnt_Typedef WWDG_Cnt40;
#endif
void TIM6_DAC_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6,TIM_IT_Update) ==SET)
	{		
		TimeBaseUpdata();
#if defined(WWDG_ON	)
		if(WWDG_Cnt40.Cnt++ >= 24)
		{
			WWDG_SetCounter(127);
			WWDG_Cnt40.Cnt = 0;
		}
#endif
		TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
	}
}


GtimerInput_Typedef Gtimer3Input = {0,0};
void TIM3_IRQHandler(void)
{ 	
	if((Gtimer3Input.gtimer_cap_sta & 0x80) == 0)//还没有成功捕获
	{
		if(TIM_GetITStatus(GTIMER_INPUT,TIM_IT_Update) == SET)
		{
			if((Gtimer3Input.gtimer_cap_sta & 0x40) == 0x40)
			{
				if((Gtimer3Input.gtimer_cap_sta & 0x3F) == 0x3F)
				{
					GTIMER_INPUT->CCER &= ~(TIM_CCER_CC1P);  
					TIM_OC1PolarityConfig(GTIMER_INPUT,TIM_ICPolarity_Rising);
					Gtimer3Input.gtimer_cap_sta |= 0x80;
					Gtimer3Input.gtimer_cap_val = 0xFFFF;
				}
				else
				{
					Gtimer3Input.gtimer_cap_sta++;
				}
			}
		}
		
		if(TIM_GetITStatus(GTIMER_INPUT,TIM_IT_CC1) == SET)
		{
			if((Gtimer3Input.gtimer_cap_sta & 0x40) == 0x40)
			{
				Gtimer3Input.gtimer_cap_sta |= 0x80;
				Gtimer3Input.gtimer_cap_val = TIM_GetCapture1(GTIMER_INPUT);
				TIM_OC1PolarityConfig(GTIMER_INPUT,TIM_OCPolarity_High);
			}
			else
			{
				Gtimer3Input.gtimer_cap_sta = 0;
				Gtimer3Input.gtimer_cap_val = 0;
				Gtimer3Input.gtimer_cap_sta |= 0x40;//捕获到了上升沿
				TIM_Cmd(GTIMER_INPUT,DISABLE);
				TIM_SetCounter(GTIMER_INPUT,0);  
				TIM_OC1PolarityConfig(GTIMER_INPUT,TIM_OCPolarity_Low);
				TIM_Cmd(GTIMER_INPUT,ENABLE);
			}
		}
	}
	TIM_ClearITPendingBit(GTIMER_INPUT,TIM_IT_Update|TIM_IT_CC1);
}





//void USART1_IRQHandler(void)
//{
//	uint16_t ReceiveData = 0;
//	
//	if(USART_GetITStatus(UART1,USART_IT_RXNE) != RESET)
//	{
//		USART_ClearITPendingBit(UART1,USART_IT_RXNE);
//		ReceiveData = USART_ReceiveData(UART1);
//		USART_SendData(UART1,ReceiveData);
//		while(USART_GetFlagStatus(UART1,USART_FLAG_TXE) == RESET);
//	}
//}
bool RTC_WKUP_Flag = 0;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_AlarmTypeDef RTC_AlarmStructure;
RTC_SetTimeStructure RTC_SetTime1;
void RTC_WKUP_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
  {
    RTC_WKUP_Flag = 1;
    RTC_ClearITPendingBit(RTC_IT_WUT);
    EXTI_ClearITPendingBit(EXTI_Line22);
  }
}

// 闹钟中断服务函数
void RTC_Alarm_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
  {    
    RTC_ClearITPendingBit(RTC_IT_ALRA);
    EXTI_ClearITPendingBit(EXTI_Line17);
  }
	printf("Time_Arrive\r\n");	
}

#if defined(WWDG_ON	)
void WWDG_IRQHandler(void)
{
	if(WWDG_GetFlagStatus() == SET)
	{
		WWDG_ClearFlag();
		WWDG_SetCounter(127);
	}
}
#endif


UartReceive_TypeDef UartBLEReceive;
void USART2_IRQHandler(void)
{
	
	uint8_t TempData = 0;
	static uint8_t state = 0;
	static uint8_t TempDataIndex = 0;
	
	if(USART_GetITStatus(UART2,USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(UART2,USART_IT_RXNE);
		TempData = USART_ReceiveData(UART2);
		if(state == 0)
		{
			if(TempData == 'S')
			{
				state = 1;
			}
		}
		else if(state == 1)
		{
			if(TempData == 'E')
			{
				state = 2;
			}
			else
			{
				UartBLEReceive.ReceiveData[TempDataIndex] = TempData;
				TempDataIndex++;
			}
		}
		else if(state == 2)
		{
			if(TempData == '$')
			{
				UartBLEReceive.ReceiveFlag = 1;
				state = 0;
				TempDataIndex = 0;
			}
		}	
	}
}






int main(int argc,char *argv[])
{ 
	float Sonic_Distance = 0;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断优先级分组
	LED_Init(CORE_LED);															//初始化核心板LED
	Button_Init(CORE_BUTTON);												//按键初始化
	System_Tick_Init();															//滴答定时器初始化
	BASE_Timer_Init(84,1000);												//基本定时器初始化,这里参数配置的1ms中断
	PWM_Config_Init(84,100);												//方波的频率为10k
	UARTx_Init(UART_1,115200);											//串口初始化UART,PA9-PA10
	UARTx_Init(UART_2,9600);												//串口初始化UART,PA9-PA10
	SONIC_GPIO_Init();															//超声波初始化
	GTIM_TIMx_Cap_Chy_Init(84,65536); 							//超声波输入捕获
	MYADC_Init();																		//ADC初始化，规则通道，开启扫描加DMA，2通道
	
	stmflash_readfull(FLASH_USER_START_ADDR,My_Flash.ReadDataTemp,FLASH_WRITE_NUM,DATA_TYPE_UINT8);
	My_RTC_Init(WKUPINT,ARARMINT);
	printf("Flash:%d,%d,%d\r\n",My_Flash.ReadDataTemp[0],My_Flash.ReadDataTemp[1],My_Flash.ReadDataTemp[2]);
	
	JSPI_Init(H_SPI);																//硬件SPI初始化
	W25QXX_SectorErase(W25QXX_SectorToErase,H_SPI);
	W25QXX_BufferWrite(Tx_Buffer, W25QXX_WriteAddress, BufferSize,H_SPI);
	W25QXX_BufferRead(Rx_Buffer, W25QXX_ReadAddress, BufferSize,H_SPI);
	printf("%s\r\n",Rx_Buffer);
#if defined(IWDG_ON)
	IWDG_Init();																		//独立看门狗1s的喂狗时间
#endif
#if defined(WWDG_ON	)
	WWDG_Init(RESET);																//窗口看门狗,喂狗时间范围21.06us-49.92ms 这里虽然算出来21.06但至少要大于它三秒左右
#endif
	printf("All Initial is OK\r\n");
	while(1)
	{	
		if(RTC_WKUP_Flag)
		{
			RTC_WKUP_Flag = 0;
			RTC_GetTime(RTC_Format_BINorBCD,&RTC_TimeStructure);
//			printf("%02d:%02d:%02d\r\n",RTC_TimeStructure.RTC_Hours,\
//			RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
		}
		
		if(TimeBaseGetFlag(BASE_100MS))
		{
			HC_SR04_Start();			
			Sonic_Distance = HC_SR04_GetDistance2();
			Get_Kernal_Tempature(&ADC_Temp,10);
//			DHT11_Calculator(Humi_Temp);//这个函数里面有大量延时函数，如果时间基准要求严格会影响到其他程序					
			
			if(UartBLEReceive.ReceiveFlag)                                         
			{
				UartBLEReceive.ReceiveFlag = 0;
				if(BLE_Modify_Time((const char*)UartBLEReceive.ReceiveData,&RTC_SetTime1) == SET)
				{
					RTC_WriteBackupRegister(RTC_BKP_DRX,FIRST_DATA);
					SetRTCAlarm(&RTC_AlarmStructure,&RTC_SetTime1);
					My_Flash.WriteData[0] = RTC_SetTime1.RTC_Hours;
					My_Flash.WriteData[1] = RTC_SetTime1.RTC_Minutes;
					My_Flash.WriteData[2] = RTC_SetTime1.RTC_Seconds;
					stmflash_write(FLASH_USER_START_ADDR,My_Flash.WriteData,FLASH_WRITE_NUM,DATA_TYPE_UINT8);
				}
			}
			TimeBaseClearFlag(BASE_100MS);
    }
		else if(TimeBaseGetFlag(BASE_1000MS))
		{
#if defined(IWDG_ON)
			IWDG_ReloadCounter();
#endif
//			printf("ADC1:%3.2fC,%d\r\n",ADC_Temp.Tempture,ADC_Temp.AdcValue[1]);
			USART_PRINT(UART2,"SonicDis:%f",Sonic_Distance);
			TimeBaseClearFlag(BASE_1000MS);
    }
	}
}




