#ifndef __HARDWARE_H_
#define __HARDWARD_H_

#include "msp432.h"

#define     LED1                  	(BIT0)
#define     LED2                  	(BIT1)
#define     LED_DIR               	(P2->DIR)
#define     LED_OUT               	(P2->OUT)

#define     BUTTON                	(BIT4)
#define     BUTTON_OUT            	(P1->OUT)
#define     BUTTON_DIR            	(P1->DIR)
#define     BUTTON_IN             	(P1->IN)
#define     BUTTON_IE             	(P1->IE)
#define     BUTTON_IES            	(P1->IES)
#define     BUTTON_IFG            	(P1->IFG)
#define     BUTTON_REN            	(P1->REN)
#define		BUTTON_IRQn				(PORT1_IRQn)
#define		BUTTON_IRQHandler		PORT1_IRQHandler


/*************************************************************
 * BLUETOOTH LOW-ENERGY HM-10
 * EUCSI_A2
 * RXD - P3.2
 * TXD - P3.3
 * EN  - P6.0
 * STATE - P6.5
 *************************************************************/

// P3.2 as RXD
#define		BT_RXD_PIN				BIT2
#define		BT_RXD_POUT				(P3->OUT)
#define 	BT_RXD_PDIR				(P3->DIR)
#define		BT_RXD_PSEL0			(P3->SEL0)
#define		BT_RXD_PSEL1			(P3->SEL1)

// P3.3 as TXD
#define		BT_TXD_PIN				BIT3
#define		BT_TXD_POUT				(P3->OUT)
#define 	BT_TXD_PDIR				(P3->DIR)
#define		BT_TXD_PSEL0			(P3->SEL0)
#define		BT_TXD_PSEL1			(P3->SEL1)

// P6.0 as ENABLE
#define		BT_EN_PIN				BIT0
#define		BT_EN_POUT				(P6->OUT)
#define 	BT_EN_PDIR				(P6->DIR)
#define		BT_EN_PSEL0				(P6->SEL0)
#define		BT_EN_PSEL1				(P6->SEL1)

// P6.5 as STATE
#define		BT_ST_PIN				BIT5
#define		BT_ST_POUT				(P6->OUT)
#define 	BT_ST_PDIR				(P6->DIR)
#define		BT_ST_PSEL0				(P6->SEL0)
#define		BT_ST_PSEL1				(P6->SEL1)

// BLE module uses EUSCI_A2
#define		BT_UART_CTL				(EUSCI_A2->CTLW0)
#define		BT_UART_BRW				(EUSCI_A2->BRW)
#define		BT_UART_MCTLW			(EUSCI_A2->MCTLW)
#define		BT_UART_IFG				(EUSCI_A2->IFG)
#define		BT_UART_IE				(EUSCI_A2->IE)
#define		BT_UART_TXD				(EUSCI_A2->TXBUF)
#define		BT_UART_RXD				(EUSCI_A2->RXBUF)
#define		BT_IRQn					EUSCIA2_IRQn
#define		BT_IRQHandler			EUSCIA2_IRQHandler

/*************************************************************
 * PC UART COMMUNICATIONS
 * EUCSI_A0
 * RXD - P1.2
 * TXD - P1.3
 *************************************************************/
// P1.2 as RXD
#define		PC_RXD_PIN				BIT2
#define		PC_RXD_POUT				(P1->OUT)
#define 	PC_RXD_PDIR				(P1->DIR)
#define		PC_RXD_PSEL0			(P1->SEL0)
#define		PC_RXD_PSEL1			(P1->SEL1)

// P1.3 as TXD
#define		PC_TXD_PIN				BIT3
#define		PC_TXD_POUT				(P1->OUT)
#define 	PC_TXD_PDIR				(P1->DIR)
#define		PC_TXD_PSEL0			(P1->SEL0)
#define		PC_TXD_PSEL1			(P1->SEL1)

// PC module uses EUSCI_A0
#define		PC_UART_CTL				(EUSCI_A0->CTLW0)
#define		PC_UART_BRW				(EUSCI_A0->BRW)
#define		PC_UART_MCTLW			(EUSCI_A0->MCTLW)
#define		PC_UART_IFG				(EUSCI_A0->IFG)
#define		PC_UART_IE				(EUSCI_A0->IE)
#define		PC_UART_TXD				(EUSCI_A0->TXBUF)
#define		PC_UART_RXD				(EUSCI_A0->RXBUF)
#define		PC_IRQn					EUSCIA0_IRQn
#define		PC_IRQHandler			EUSCIA0_IRQHandler

// Function Prototypes
void	lowPowerConfig(void);	//Place Ports in lowest power configuration

#endif	//__HARDWARE_H_
