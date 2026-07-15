#include <stdio.h>
#include <string.h>
#include "xpt2046.h"
#include "delay.h"

// 全局校准极值变量
int lX_Min = 200;
int lX_Max = 3800;
int lY_Min = 200;
int lY_Max = 3800;

// 通道命令宏
#define XPT2046_CHANNEL_X 0x90 //通道Y+
#define XPT2046_CHANNEL_Y 0xd0 //通道X+

#define TOUCH_PRESSED 1
#define TOUCH_NOT_PRESSED 0

#define XPT2046_PENIRQ_Active 0
#define XPT2046_THRESHOLD_CalDiff 2
#define DURIATION_TIME 2

// IO操作宏（全部放在函数外部）
#define XPT2046_CS_ENABLE()    GPIO_ResetBits(GPIOD, GPIO_Pin_13)
#define XPT2046_CS_DISABLE()   GPIO_SetBits(GPIOD, GPIO_Pin_13)

#define XPT2046_CLK_HIGH()     GPIO_SetBits(GPIOE, GPIO_Pin_0)
#define XPT2046_CLK_LOW()      GPIO_ResetBits(GPIOE, GPIO_Pin_0)

#define XPT2046_MOSI_1()       GPIO_SetBits(GPIOE, GPIO_Pin_2)
#define XPT2046_MOSI_0()       GPIO_ResetBits(GPIOE, GPIO_Pin_2)

#define XPT2046_MISO()         GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)
#define XPT2046_ReadIRQ()      GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)

#define XPT2046_DelayUS(x)     delay_us(x)

int LCD_SCAN_MODE = 0;

// 屏幕校准系数表
XPT2046_Factor TouchFactor[] = {
    {-0.006464, -0.073259, 280.358032, 0.074878, 0.002052, -6.545977},  //0
    {0.086314, 0.001891, -12.836658, -0.003722, -0.065799, 254.715714}, //1
    {0.002782, 0.061522, -11.595689, 0.083393, 0.005159, -15.650089},  //2
    {0.089743, -0.000289, -20.612209, -0.001374, 0.064451, -16.054003}, //3
    {0.000767, -0.068258, 250.891769, -0.085559, -0.000195, 334.747650}, //4
    {-0.084744, 0.000047, 323.163147, -0.002109, -0.066371, 260.985809}, //5
    {-0.001848, 0.066984, -12.807136, -0.084858, -0.000805, 333.395386}, //6
    {-0.085470, -0.000876, 334.023163, -0.003390, 0.064725, -6.211169},  //7
};

/**
* @brief XPT2046 初始化函数
* @param 无
* @retval 无
*/
void XPT2046_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

    // CLK PE0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // MOSI PE2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // CS PD13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // MISO PE3 上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // IRQ PE4 上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 空闲拉高片选
    XPT2046_CS_DISABLE();
}

/**
* @brief XPT2046 写入命令
* @param ucCmd 命令字
* @retval 无
*/
static void XPT2046_WriteCMD(uint8_t ucCmd)
{
    uint8_t i = 0;
    XPT2046_MOSI_0();
    XPT2046_CLK_LOW();

    for(i = 0; i < 8; i++)
    {
        ((ucCmd >> (7 - i)) & 0x01) ? XPT2046_MOSI_1() : XPT2046_MOSI_0();
        XPT2046_DelayUS(5);
        XPT2046_CLK_HIGH();
        XPT2046_DelayUS(5);
        XPT2046_CLK_LOW();
    }
}

/**
* @brief XPT2046 读取12位ADC数据
* @retval ADC原始值
*/
static uint16_t XPT2046_ReadCMD(void)
{
    uint8_t i;
    uint16_t usBuf = 0, usTemp;
    XPT2046_MOSI_0();
    XPT2046_CLK_HIGH();

    for(i = 0; i < 12; i++)
    {
        XPT2046_CLK_LOW();
        usTemp = XPT2046_MISO();
        usBuf |= usTemp << (11 - i);
        XPT2046_CLK_HIGH();
    }
    return usBuf;
}

/**
* @brief 读取单通道ADC
* @param ucChannel 通道命令
* @retval ADC值
*/
static uint16_t XPT2046_ReadAdc(uint8_t ucChannel)
{
    XPT2046_CS_ENABLE();
    XPT2046_WriteCMD(ucChannel);
    uint16_t dat = XPT2046_ReadCMD();
    XPT2046_CS_DISABLE();
    return dat;
}

/**
* @brief 同时读取X、Y原始AD值
* @param sX_Ad X坐标指针
* @param sY_Ad Y坐标指针
* @retval 无
*/
static void XPT2046_ReadAdcXY(uint16_t *sX_Ad, uint16_t *sY_Ad)
{
    *sX_Ad = XPT2046_ReadAdc(XPT2046_CHANNEL_X);
    XPT2046_DelayUS(1);
    *sY_Ad = XPT2046_ReadAdc(XPT2046_CHANNEL_Y);
}

/**
* @brief 多点采样滤波，剔除极值取平均
* @param pCoord 输出滤波后原始AD坐标
* @retval 0成功 -1失败
*/
static int8_t XPT2046_FilterXY(XPT2046_Coord *pCoord)
{
    uint8_t ucCount = 0, i = 0;
    uint16_t value = 0;
    uint16_t tempAD_X, tempAD_Y = 0;
    uint16_t sBufferArray[2][10] = {{0}, {0}};

    do
    {
        XPT2046_ReadAdcXY(&tempAD_X, &tempAD_Y);
        sBufferArray[0][ucCount] = tempAD_X;
        sBufferArray[1][ucCount] = tempAD_Y;
        ucCount++;
        value = XPT2046_ReadIRQ();
    } while ((value == XPT2046_PENIRQ_Active) && (ucCount < 10));

    if (ucCount < 10)
    {
        return -1;
    }

    // X轴极值与求和
    tempAD_X = sBufferArray[0][0];
    lX_Max = lX_Min = sBufferArray[0][0];
    for (i = 1; i < 10; i++)
    {
        if (sBufferArray[0][i] < lX_Min)
            lX_Min = sBufferArray[0][i];
        else if (sBufferArray[0][i] > lX_Max)
            lX_Max = sBufferArray[0][i];
        tempAD_X += sBufferArray[0][i];
    }

    // Y轴极值与求和
    tempAD_Y = sBufferArray[1][0];
    lY_Max = lY_Min = sBufferArray[1][0];
    for (i = 1; i < 10; i++)
    {
        if (sBufferArray[1][i] < lY_Min)
            lY_Min = sBufferArray[1][i];
        else if (sBufferArray[1][i] > lY_Max)
            lY_Max = sBufferArray[1][i];
        tempAD_Y += sBufferArray[1][i];
    }

    // 剔除最大最小8点平均
    pCoord->x = (tempAD_X - lX_Min - lX_Max) / 8;
    pCoord->y = (tempAD_Y - lY_Min - lY_Max) / 8;

    return 0;
}

/**
* @brief 获取校准后屏幕坐标
* @param pLCD 坐标结构体指针
* @retval 0成功 -1失败
*/
int8_t XPT2046_GetTouchPoint(XPT2046_Coord *pLCD)
{
    XPT2046_Coord adcCoord;
    if (XPT2046_FilterXY(&adcCoord) < 0)
        return -1;

    pLCD->x = TouchFactor[LCD_SCAN_MODE].A * adcCoord.x +
              TouchFactor[LCD_SCAN_MODE].B * adcCoord.y +
              TouchFactor[LCD_SCAN_MODE].C;

    pLCD->y = TouchFactor[LCD_SCAN_MODE].D * adcCoord.x +
              TouchFactor[LCD_SCAN_MODE].E * adcCoord.y +
              TouchFactor[LCD_SCAN_MODE].F;
    return 0;
}

/**
* @brief 触摸按下回调
*/
__weak void XPT2046_TouchDown(XPT2046_Coord *touch)
{
    printf("TouchDown,x:%d,y:%d\r\n", touch->x, touch->y);
}

/**
* @brief 触摸释放回调
*/
__weak void XPT2046_TouchUp(XPT2046_Coord *touch)
{
    printf("TouchUp,x:%d,y:%d\r\n", touch->x, touch->y);
}

/**
* @brief 触摸状态机检测
* @retval TOUCH_PRESSED / TOUCH_NOT_PRESSED
*/
uint8_t XPT2046_DetectTouch(void)
{
    uint8_t detectResult = TOUCH_NOT_PRESSED;
    uint16_t value = 0;
    static uint32_t i = 0;
    static enumTouchState touch_state = XPT2046_STATE_RELEASE;
    static XPT2046_Coord cinfo = {-1, -1, -1, -1};

    value = XPT2046_ReadIRQ();
    switch (touch_state)
    {
        case XPT2046_STATE_RELEASE:
            if (value == XPT2046_PENIRQ_Active)
            {
                touch_state = XPT2046_STATE_WAITING;
                detectResult = TOUCH_NOT_PRESSED;
            }
            break;
        case XPT2046_STATE_WAITING:
            if (value == XPT2046_PENIRQ_Active)
            {
                i++;
                if (i > DURIATION_TIME)
                {
                    i = 0;
                    touch_state = XPT2046_STATE_PRESSED;
                    detectResult = TOUCH_PRESSED;
                }
            }
            else
            {
                i = 0;
                touch_state = XPT2046_STATE_RELEASE;
            }
            break;
        case XPT2046_STATE_PRESSED:
            if (value != XPT2046_PENIRQ_Active)
            {
                touch_state = XPT2046_STATE_RELEASE;
                detectResult = TOUCH_NOT_PRESSED;
            }
            else
            {
                detectResult = TOUCH_PRESSED;
            }
            break;
        default:
            touch_state = XPT2046_STATE_RELEASE;
            break;
    }

    if (detectResult == TOUCH_PRESSED)
    {
        if (XPT2046_GetTouchPoint(&cinfo) < 0)
            return detectResult;
        XPT2046_TouchDown(&cinfo);
        cinfo.pre_x = cinfo.x;
        cinfo.pre_y = cinfo.y;
    }
    else
    {
        // 修复：判断x、y同时为-1
        if (cinfo.pre_x == -1 && cinfo.pre_y == -1)
            return detectResult;

        XPT2046_TouchUp(&cinfo);
        cinfo.x = -1;
        cinfo.y = -1;
        cinfo.pre_x = -1;
        cinfo.pre_y = -1;
    }
    return detectResult;
}