#include "stm32f10x.h"

// IIC引脚定义
#define IIC_SCL_PIN    GPIO_Pin_6
#define IIC_SDA_PIN    GPIO_Pin_7
#define IIC_GPIO       GPIOB
#define IIC_RCC        RCC_APB2Periph_GPIOB

// IIC操作函数
static void IIC_Delay(void)
{
    uint8_t i = 4; // 延时时间可根据实际时钟频率调整
    while(i--);
}

// SDA输出模式配置
static void SDA_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IIC_GPIO, &GPIO_InitStructure);
}

// SDA输入模式配置
static void SDA_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(IIC_GPIO, &GPIO_InitStructure);
}

// IIC起始信号
void IIC_Start(void)
{
    SDA_OUT();
    GPIO_SetBits(IIC_GPIO, IIC_SDA_PIN);
    GPIO_SetBits(IIC_GPIO, IIC_SCL_PIN);
    IIC_Delay();
    GPIO_ResetBits(IIC_GPIO, IIC_SDA_PIN);
    IIC_Delay();
    GPIO_ResetBits(IIC_GPIO, IIC_SCL_PIN);
}

// IIC停止信号
void IIC_Stop(void)
{
    SDA_OUT();
    GPIO_ResetBits(IIC_GPIO, IIC_SCL_PIN);
    GPIO_ResetBits(IIC_GPIO, IIC_SDA_PIN);
    IIC_Delay();
    GPIO_SetBits(IIC_GPIO, IIC_SCL_PIN);
    GPIO_SetBits(IIC_GPIO, IIC_SDA_PIN);
    IIC_Delay();
}

// IIC等待应答
uint8_t IIC_Wait_Ack(void)
{
    uint8_t timeout = 0;
    
    SDA_IN();
    GPIO_SetBits(IIC_GPIO, IIC_SDA_PIN);
    IIC_Delay();
    GPIO_SetBits(IIC_GPIO, IIC_SCL_PIN);
    IIC_Delay();
    
    while(GPIO_ReadInputDataBit(IIC_GPIO, IIC_SDA_PIN))
    {
        timeout++;
        if(timeout > 250)
        {
            IIC_Stop();
            return 1;
        }
    }
    GPIO_ResetBits(IIC_GPIO, IIC_SCL_PIN);
    return 0;
}

// IIC发送一个字节
void IIC_Send_Byte(uint8_t data)
{
    uint8_t i;
    SDA_OUT();
    GPIO_ResetBits(IIC_GPIO, IIC_SCL_PIN);
    for(i = 0; i < 8; i++)
    {
        if((data & 0x80) >> 7)
            GPIO_SetBits(IIC_GPIO, IIC_SDA_PIN);
        else
            GPIO_ResetBits(IIC_GPIO, IIC_SDA_PIN);
        data <<= 1;
        IIC_Delay();
        GPIO_SetBits(IIC_GPIO, IIC_SCL_PIN);
        IIC_Delay();
        GPIO_ResetBits(IIC_GPIO, IIC_SCL_PIN);
        IIC_Delay();
    }
}

// IIC读取一个字节
uint8_t IIC_Read_Byte(uint8_t ack)
{
    uint8_t i, receive = 0;
    SDA_IN();
    for(i = 0; i < 8; i++)
    {
        GPIO_ResetBits(IIC_GPIO, IIC_SCL_PIN);
        IIC_Delay();
        GPIO_SetBits(IIC_GPIO, IIC_SCL_PIN);
        receive <<= 1;
        if(GPIO_ReadInputDataBit(IIC_GPIO, IIC_SDA_PIN))
            receive++;
        IIC_Delay();
    }
    if(!ack)
        IIC_NAK();
    else
        IIC_ACK();
    return receive;
}

// IIC初始化
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(IIC_RCC, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = IIC_SCL_PIN | IIC_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IIC_GPIO, &GPIO_InitStructure);
    
    GPIO_SetBits(IIC_GPIO, IIC_SCL_PIN | IIC_SDA_PIN);
}
