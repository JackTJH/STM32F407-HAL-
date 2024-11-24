#ifndef __JACK_I2C_H
#define __JACK_I2C_H


#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx.h"
 
typedef enum
{
	H_I2C,
	S_I2C
}	I2C_TYPE;

typedef enum
{
	JI2C1,
	JI2C2
} I2CX_Typedef;

#define I2C1MODE												H_I2C

#define I2Cn														2

#define I2C1_GPIO_RCC										RCC_AHB1Periph_GPIOB
#define I2C2_GPIO_RCC										RCC_AHB1Periph_GPIOB

#define I2C1_GPIO_Port									GPIOB
#define I2C2_GPIO_Port									GPIOB

#define I2C1_SCL_GPIO_Pin								GPIO_Pin_8
#define I2C1_SDA_GPIO_Pin								GPIO_Pin_9
#define I2C2_SCL_GPIO_Pin								GPIO_Pin_10
#define I2C2_SDA_GPIO_Pin								GPIO_Pin_11

#define I2C1_SCL_GPIO_AFPin							GPIO_PinSource8
#define I2C1_SDA_GPIO_AFPin							GPIO_PinSource9
#define I2C2_SCL_GPIO_AFPin							GPIO_PinSource10
#define I2C2_SDA_GPIO_AFPin							GPIO_PinSource11

#define I2C1_GPIO_AF										GPIO_AF_I2C1
#define I2C2_GPIO_AF										GPIO_AF_I2C2

#define I2C1_GPIO_CLK_ENABLE()					RCC_AHB1PeriphClockCmd(I2C1_GPIO_RCC, ENABLE)
#define I2C2_GPIO_CLK_ENABLE()					RCC_AHB1PeriphClockCmd(I2C2_GPIO_RCC, ENABLE)

#define I2CX_GPIO_CLK_ENABLE(__INDEX__) do{if     ((__INDEX__) == 0) I2C1_GPIO_CLK_ENABLE();\
																					 else if((__INDEX__) == 1) I2C2_GPIO_CLK_ENABLE();\
																					}while(0)

#define I2CX_CLK_ENABLE(__INDEX__) 			do{if     ((__INDEX__) == 0) RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);\
																					 else if((__INDEX__) == 1) RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);\
																					}while(0)


#define I2C1_SCL(bit)										GPIO_WriteBit(I2C1_GPIO_Port,I2C1_SCL_GPIO_Pin,(bit)?Bit_SET:Bit_RESET)
#define I2C1_SDA(bit)										GPIO_WriteBit(I2C1_GPIO_Port,I2C1_SDA_GPIO_Pin,(bit)?Bit_SET:Bit_RESET)
#define	I2C1_ReadSDA()									GPIO_ReadInputDataBit(I2C1_GPIO_Port,I2C1_SDA_GPIO_Pin)
																					
#define I2C2_SCL(bit)										GPIO_WriteBit(I2C2_GPIO_Port,I2C2_SCL_GPIO_Pin,(bit)?Bit_SET:Bit_RESET)
#define I2C2_SDA(bit)										GPIO_WriteBit(I2C2_GPIO_Port,I2C2_SDA_GPIO_Pin,(bit)?Bit_SET:Bit_RESET)
#define	I2C2_ReadSDA()									GPIO_ReadInputDataBit(I2C2_GPIO_Port,I2C2_SDA_GPIO_Pin)
																					
extern I2C_TypeDef* JI2X[I2Cn];
	 
void JI2C_Init(I2C_TYPE i2c_type,I2CX_Typedef i2cx);
																					
void JI2C_Start(I2CX_Typedef i2cx);
void JI2C_Stop(I2CX_Typedef i2cx);
																					
uint8_t JI2C_WaitAck(I2CX_Typedef i2cx);
void JI2C_MasterAck(uint8_t ack, I2CX_Typedef i2cx);
																					
void JI2C_SendByte(uint8_t data,I2CX_Typedef i2cx);
uint8_t JI2C_ReadByte(I2CX_Typedef i2cx);			

void JI2CXX_WaitEvent(I2CX_Typedef i2cx, uint32_t I2C_EVENT);																					
	 
#ifdef __cplusplus
}
#endif

#endif
