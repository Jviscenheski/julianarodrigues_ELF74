#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#define GPIO_PA0_U0RX   0x00000001
#define GPIO_PA1_U0TX   0x00000401
#define BUFFER_SIZE     4 

uint8_t buffer[BUFFER_SIZE];
uint8_t bufferPosition;
uint8_t finalPosition;


void UART_Interruption_Handler(void){
  // fun��o que lida com a interrup��o gerada pela entrada de dados 
  // na comunica��o serial
  
  // armazena no buffer a entrada
  buffer[bufferPosition%BUFFER_SIZE] = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
  
  // incrementa a posi��o do buffer
  bufferPosition++;
  
  // limpa o buffer
  UARTIntClear(UART0_BASE,UARTIntStatus(UART0_BASE,true));
}

int main()
{
  // ativa o perif�rico de entrada de dados
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  // aguarda enquanto o perif�rico n�o estiver ok
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
  
  // configura��o dos perif�ricos
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  
  // ativa a entrada serial
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  // aguarda enquanto a entrada serial n�o estiver ok
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));  
  
  // define e configura as portas seriais
  GPIOPinTypeUART(GPIO_PORTA_BASE,(GPIO_PIN_0|GPIO_PIN_1));
  UARTConfigSetExpClk(UART0_BASE,(uint32_t)120000000,(uint32_t)115200,
               (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));
  UARTFIFODisable(UART0_BASE);
  UARTIntEnable(UART0_BASE,UART_INT_RX|UART_INT_RT);
  UARTIntRegister(UART0_BASE,UART_Interruption_Handler);
  
  bufferPosition = 0;
  finalPosition = 0;
  uint8_t *wordAux;

  while(1){
    
    // se ainda n�o chegou ao final da palavra
    if(bufferPosition != finalPosition){
      
      // salva o que est� no buffer em uma vari�vel auxiliar
      wordAux = &buffer[finalPosition%BUFFER_SIZE];   
      
      // se o dado recebido for uma letra mai�scula, transforma em min�scula
      // faz as rota��es no c�digo ASCII
      if(((*wordAux)>=(uint8_t)65)&&((*wordAux)<=(uint8_t)90)){
        *wordAux += 32; 
      }
      
      if(UARTCharPutNonBlocking(UART0_BASE,buffer[finalPosition%BUFFER_SIZE])){
        finalPosition++;
      }
    }      
  }
}
