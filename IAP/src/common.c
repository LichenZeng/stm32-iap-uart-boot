/**
  ******************************************************************************
  * @file    IAP/src/common.c
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    10/15/2010
  * @brief   This file provides all the common functions.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */

/** @addtogroup IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include <string.h>
#include <stdlib.h>

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
  * @brief  Configures COM port.
  * @param  COM: Specifies the COM port to be configured.
  *   This parameter can be one of following parameters:
  *     @arg COM1
  *     @arg COM2
  * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that
  *   contains the configuration information for the specified USART peripheral.
  * @retval None
  */
void STM_EVAL_COMInit(USART_InitTypeDef* USART_InitStruct)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIO clock */
    RCC_APB2PeriphClockCmd(EVAL_COM2_TX_GPIO_CLK | EVAL_COM2_RX_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);

    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(EVAL_COM2_CLK, ENABLE);


    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = EVAL_COM2_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(EVAL_COM2_TX_GPIO_PORT, &GPIO_InitStructure);

    /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = EVAL_COM2_RX_PIN;
    GPIO_Init(EVAL_COM2_RX_GPIO_PORT, &GPIO_InitStructure);

    /* USART configuration */
    USART_Init(EVAL_COM2, USART_InitStruct);

    /* Enable USART */
    USART_Cmd(EVAL_COM2, ENABLE);
}


/**
  * @brief  Convert an Integer to a string
  * @param  str: The string
  * @param  intnum: The intger to be converted
  * @retval None
  */
void Int2Str(uint8_t* str, int32_t intnum)
{
    uint32_t i, Div = 1000000000, j = 0, Status = 0;

    for (i = 0; i < 10; i++)
    {
        str[j++] = (intnum / Div) + 48;

        intnum = intnum % Div;
        Div /= 10;
        if ((str[j-1] == '0') & (Status == 0))
        {
            j = 0;
        }
        else
        {
            Status++;
        }
    }
    str[j] = '\0';
}

/**
  * @brief  Convert a string to an integer
  * @param  inputstr: The string to be converted
  * @param  intnum: The intger value
  * @retval 1: Correct
  *         0: Error
  */
uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum)
{
    uint32_t i = 0, res = 0;
    uint32_t val = 0;

    if (inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X'))
    {
        if (inputstr[2] == '\0')
        {
            return 0;
        }
        for (i = 2; i < 11; i++)
        {
            if (inputstr[i] == '\0')
            {
                *intnum = val;
                /* return 1; */
                res = 1;
                break;
            }
            if (ISVALIDHEX(inputstr[i]))
            {
                val = (val << 4) + CONVERTHEX(inputstr[i]);
            }
            else
            {
                /* return 0, Invalid input */
                res = 0;
                break;
            }
        }
        /* over 8 digit hex --invalid */
        if (i >= 11)
        {
            res = 0;
        }
    }
    else /* max 10-digit decimal input */
    {
        for (i = 0; i < 11; i++)
        {
            if (inputstr[i] == '\0')
            {
                *intnum = val;
                /* return 1 */
                res = 1;
                break;
            }
            else if ((inputstr[i] == 'k' || inputstr[i] == 'K') && (i > 0))
            {
                val = val << 10;
                *intnum = val;
                res = 1;
                break;
            }
            else if ((inputstr[i] == 'm' || inputstr[i] == 'M') && (i > 0))
            {
                val = val << 20;
                *intnum = val;
                res = 1;
                break;
            }
            else if (ISVALIDDEC(inputstr[i]))
            {
                val = val * 10 + CONVERTDEC(inputstr[i]);
            }
            else
            {
                /* return 0, Invalid input */
                res = 0;
                break;
            }
        }
        /* Over 10 digit decimal --invalid */
        if (i >= 11)
        {
            res = 0;
        }
    }

    return res;
}

/**
  * @brief  Get an integer from the HyperTerminal
  * @param  num: The inetger
  * @retval 1: Correct
  *         0: Error
  */
uint32_t GetIntegerInput(int32_t * num)
{
    uint8_t inputstr[16];

    while (1)
    {
        GetInputString(inputstr);
        if (inputstr[0] == '\0') continue;
        if ((inputstr[0] == 'a' || inputstr[0] == 'A') && inputstr[1] == '\0')
        {
            SerialPutString(" User Cancelled.\r\n");
            return 0;
        }

        if (Str2Int(inputstr, num) == 0)
        {
            SerialPutString(" Error, Input.\r\n");
        }
        else
        {
            return 1;
        }
    }
}

/**
  * @brief  Test to see if a key has been pressed on the HyperTerminal
  * @param  key: The key pressed
  * @retval 1: Correct
  *         0: Error
  */
uint32_t SerialKeyPressed(uint8_t *key)
{

    if ( USART_GetFlagStatus(EVAL_COM2, USART_FLAG_RXNE) != RESET)
    {
        *key = (uint8_t)EVAL_COM2->DR;
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
  * @brief  Get a key from the HyperTerminal
  * @param  None
  * @retval The Key Pressed
  */
uint8_t GetKey(void)
{
    uint8_t key = 0;
    /* Waiting for user input */
    while (1)
    {
        if (SerialKeyPressed((uint8_t*)&key)) break;
    }
    return key;

}

/**
  * @brief  Print a character on the HyperTerminal
  * @param  c: The character to be printed
  * @retval None
  */
void SerialPutChar(uint8_t c)
{
    USART_SendData(EVAL_COM2, c);
    while (USART_GetFlagStatus(EVAL_COM2, USART_FLAG_TXE) == RESET)
    {
    }
}

/**
  * @brief  Print a string on the HyperTerminal
  * @param  s: The string to be printed
  * @retval None
  */
void Serial_PutString(uint8_t *s)
{
#if (ENABLE_PUTSTR == 1)
    while (*s != '\0')
    {
        SerialPutChar(*s);
        s++;
    }
#endif
}

/**
  * @brief  Get Input string from the HyperTerminal
  * @param  buffP: The input string
  * @retval None
  */
void GetInputString (uint8_t * buffP)
{
    uint32_t bytes_read = 0;
    uint8_t c = 0;
    do
    {
        //SerialPutChar('a');
        c = GetKey();
        /* Note: Windows is \r\n */
        if (c == '\n' | c == '\r')
        {
            //SerialPutChar('n');
            if(buffP[bytes_read-1] == '\r')
                break;
        }

        /* Note: \b = 8, but DEL valuse is 127 */
        if (c == '\b' || c == 127) /* Backspace */
        {
            if (bytes_read > 0)
            {
                SerialPutChar(127);
                //SerialPutString("\b \b");
                bytes_read --;
            }
            continue;
        }
        if (bytes_read >= CMD_STRING_SIZE )
        {
            SerialPutString(" Cmd size over.\r\n");
            bytes_read = 0;
            continue;
        }
        if ((c >= 0x20 && c <= 0x7E) || c == '\r')
        {
            buffP[bytes_read++] = c;
            SerialPutChar(c);
        }
    }
    while (1);
    SerialPutString(("\r\n"));
    buffP[bytes_read-1] = '\0';
    //SerialPutString(buffP);
}




/**
  * @}
  */

/**
  * @brief  Calculate the number of pages
  * @param  Size: The image size
  * @retval The number of pages
  */
uint32_t FLASH_PagesMask(__IO uint32_t Size)
{
    uint32_t pagenumber = 0x0;
    uint32_t size = Size;

    if ((size % PAGE_SIZE) != 0)
    {
        pagenumber = (size / PAGE_SIZE) + 1;
    }
    else
    {
        pagenumber = size / PAGE_SIZE;
    }
    return pagenumber;
}

uint8_t EraseSomePages(__IO uint32_t size, uint8_t outPutCont)
{
    uint32_t EraseCounter = 0x0;
    uint32_t NbrOfPage = 0;
    uint8_t erase_cont[3] = {0};
    FLASH_Status FLASHStatus = FLASH_COMPLETE;

    NbrOfPage = FLASH_PagesMask(size);

    /* Erase the FLASH pages */
    FLASH_Unlock();
    for (EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        FLASHStatus = FLASH_ErasePage(ApplicationAddress + (PAGE_SIZE * EraseCounter));
        if(outPutCont == 1)
        {
            Int2Str(erase_cont, EraseCounter + 1);
            SerialPutString(erase_cont);
            SerialPutString("@");
        }
    }
    FLASH_Lock();
    if((EraseCounter != NbrOfPage) || (FLASHStatus != FLASH_COMPLETE))
    {
        return 0;
    }
    return 1;
}

/**
 * @file   Delay_ms
 * @brief  毫秒延时time_ms ms
 * @param   time_ms 延时时间
 * @retval 无
 */
void Delay_ms( uint16_t time_ms )
{
    uint16_t i,j;
    for( i=0; i<time_ms; i++ )
    {
        for( j=0; j<4784; j++ );
    }
}
/*******************(C)COPYRIGHT 2010 STMicroelectronics *****END OF FILE******/
