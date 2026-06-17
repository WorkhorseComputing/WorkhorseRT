/** MIT License
 *
 * Copyright (c) 2026 Humza Khan
 * <mohammed.khan.2024@uni.strath.ac.uk>
 * <https://github.com/humzak711>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef _IA32E_UART_H_
#define _IA32E_UART_H_

#include <ia32eAsm.h>
#include <errno.h>

#define IA32E_UART_COM1_PORT                        0x3F8
#define IA32E_UART_COM2_PORT                        0x2F8
#define IA32E_UART_COM3_PORT                        0x3E8
#define IA32E_UART_COM4_PORT                        0x2E8
#define IA32E_UART_COM5_PORT                        0x5F8
#define IA32E_UART_COM6_PORT                        0x4F8
#define IA32E_UART_COM7_PORT                        0x5E8
#define IA32E_UART_COM8_PORT                        0x4E8
#define IA32E_UART_COM_PORT                         IA32E_UART_COM1_PORT

#define IA32E_UART_RECV_BUF_OFFSET_R                0
#define IA32E_UART_TRANSMIT_BUF_OFFSET_W            0
#define IA32E_UART_INT_ENABLE_OFFSET_RW             1
#define IA32E_UART_DIVISOR_LSB_OFFSET_RW            0
#define IA32E_UART_DIVISOR_MSB_OFFSET_RW            1
#define IA32E_UART_INT_ID_OFFSET_R                  2
#define IA32E_UART_FIFO_CTRL_OFFSET_W               2
#define IA32E_UART_LINE_CTRL_OFFSET_RW              3
#define IA32E_UART_MODEM_CTRL_OFFSET_RW             4
#define IA32E_UART_LINE_STATUS_OFFSET_R             5
#define IA32E_UART_MODEM_STATUS_OFFSET_R            6
#define IA32E_UART_SCRATCH_OFFSET_RW                7

#define IA32E_UART_CHAR_LEN_5                       0
#define IA32E_UART_CHAR_LEN_6                       1
#define IA32E_UART_CHAR_LEN_7                       2
#define IA32E_UART_CHAR_LEN_8                       3

#define IA32E_UART_INT_TRIGGER_LEVEL_1              0
#define IA32E_UART_INT_TRIGGER_LEVEL_4              1
#define IA32E_UART_INT_TRIGGER_LEVEL_8              2
#define IA32E_UART_INT_TRIGGER_LEVEL_14             3

#define IA32E_UART_MODEM_STATUS_INT                 0
#define IA32E_UART_TRANS_HOLDING_EMPTY_INT          1
#define IA32E_UART_RECV_DATA_AVAILABLE_INT          2
#define IA32E_UART_RECEIVER_LINE_STATUS_INT         3

#define IA32E_UART_MODEM_STATUS_PRIORITY            4
#define IA32E_UART_TRANS_HOLDING_EMPTY_PRIORITY     3
#define IA32E_UART_RECV_DATA_AVAILABLE_PRIORITY     2
#define IA32E_UART_RECEIVER_LINE_STATUS_PRIORITY    1

#define IA32E_UART_FIFO_BUF_STATE_NONE              0
#define IA32E_UART_FIFO_BUF_STATE_UNUSABLE          1
#define IA32E_UART_FIFO_BUF_STATE_ENABLED           2

inline 
bool __ia32eUartIsReceived(uint16_t com_port)
{
    uint8_t status = __ia32eInb(com_port + IA32E_UART_LINE_STATUS_OFFSET_R);
    return (status & 1) != 0;
}

inline 
char __ia32UartRecv(uint16_t com_port)
{
    spinUntil(__ia32eUartIsReceived(com_port));
    return __ia32eInb(com_port);
}

inline 
bool __ia32eUartIsTransmitEmpty(uint16_t com_port)
{
    uint8_t status = __ia32eInb(com_port + IA32E_UART_LINE_STATUS_OFFSET_R);
    return (status & (1 << 5)) != 0;   
}

inline 
void __ia32eUartTransmit(uint16_t com_port, char val)
{
    spinUntil(__ia32eUartIsTransmitEmpty(com_port));
    __ia32eOutb(com_port, val);
}

inline 
void __ia32eUartTransmitStr(uint16_t com_port, const char *str)
{
    while (*str != '\0') 
        __ia32eUartTransmit(com_port, *str++);
}

inline 
int __ia32eUartInit(uint16_t com_port)
{
    __ia32eOutb(com_port + IA32E_UART_INT_ENABLE_OFFSET_RW, 0x0);

    /* Set the baud rate */

    __ia32eOutb(com_port + IA32E_UART_LINE_CTRL_OFFSET_RW, 0x80);
    __ia32eOutb(com_port + IA32E_UART_DIVISOR_LSB_OFFSET_RW, 0x3);
    __ia32eOutb(com_port + IA32E_UART_DIVISOR_MSB_OFFSET_RW, 0x0);
    __ia32eOutb(com_port + IA32E_UART_LINE_CTRL_OFFSET_RW, 0x3);

    /* Enable fifos */
   
    __ia32eOutb(com_port + IA32E_UART_FIFO_CTRL_OFFSET_W, 0xc7);

    /* Loopback test to check the device isnt fucked up */

    __ia32eOutb(com_port + IA32E_UART_MODEM_CTRL_OFFSET_RW, 0x1e);
    __ia32eOutb(com_port, TEST_BYTE);

    if (__ia32eInb(com_port) != TEST_BYTE)
        return -EIO;

    /* Set modem ctrl into regular operation, we know shit works */

    __ia32eOutb(com_port + IA32E_UART_MODEM_CTRL_OFFSET_RW, 0x1);

    return 0;
}

void ia32eUartPrint(const char *str);
void ia32eUartInit(void);

#endif