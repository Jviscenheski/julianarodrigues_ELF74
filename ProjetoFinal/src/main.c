#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "inc/hw_memmap.h"
#include "tx_api.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "system_TM4C1294.h"
#include "utils/uartstdio.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"


#define BUFFER_SIZE 16
#define DEMO_BYTE_POOL_SIZE     9120
#define DEMO_STACK_SIZE         1024
#define DEMO_QUEUE_SIZE         100
#define GPIO_PA0_U0RX           0x00000001
#define GPIO_PA1_U0TX           0x00000401

UCHAR                   byte_pool_memory[DEMO_BYTE_POOL_SIZE];
TX_THREAD               thread_1;
TX_THREAD               thread_2;
TX_THREAD               thread_3;
TX_THREAD               thread_main;
TX_BYTE_POOL            byte_pool_0;
TX_QUEUE                uart_left;
TX_QUEUE                uart_center;
TX_QUEUE                uart_right;
TX_QUEUE                intern_left;
TX_QUEUE                intern_center;
TX_QUEUE                intern_right;
TX_MUTEX                mutex_uart;


void receive_commands_from_user(ULONG aux){
  
  int buf_pos=0;
  char received_command[8];
  char uart_char;
  int completed_command_from_uart = 0;
  UINT status;

  while(1){
      
     while(UARTCharsAvail(UART0_BASE)){
      uart_char = UARTCharGet(UART0_BASE);
      if(uart_char!='\n' && uart_char!='\r'){
       received_command[buf_pos] = uart_char;
       buf_pos++;
        if(uart_char=='F' || uart_char=='A'){
          memset(received_command, 0, sizeof received_command);
          buf_pos=0;
        }
      }
      else{
        buf_pos=0;
        completed_command_from_uart=1;
        }
      tx_thread_sleep(2);
     }
      
      if(completed_command_from_uart){
        if(received_command[0]=='e'){
          status = tx_queue_send(&uart_left, received_command, TX_WAIT_FOREVER);
          if (status != TX_SUCCESS)
              break;
        }
        if(received_command[0]=='c'){
          status = tx_queue_send(&uart_center, received_command, TX_WAIT_FOREVER);
          if (status != TX_SUCCESS)
              break;
        }
        if(received_command[0]=='d'){
          status = tx_queue_send(&uart_right, received_command, TX_WAIT_FOREVER);
          if (status != TX_SUCCESS)
              break;
        }

        memset(received_command, 0, sizeof received_command);
        completed_command_from_uart=0;
      }
     
      tx_thread_sleep(1);
    }
}

char floor_char_from_decimal_and_unit(char primeiro, char segundo){
  int floor = (primeiro - '0') * 10 + (segundo - '0');
  
  switch (floor) {
      case 0:
          return 'a';
      case 1:
          return 'b';
      case 2:
          return 'c';
      case 3:
          return 'd';
      case 4:
          return 'e';
      case 5:
          return 'f';
      case 6:
          return 'g';
      case 7:
          return 'h';
      case 8:
          return 'i';
      case 9:
          return 'j';
      case 10:
          return 'k';
      case 11:
          return 'l';
      case 12:
          return 'm';
      case 13:
          return 'n';
      case 14:
          return 'o';
      case 15:
          return 'p';
      default:
          return ' ';
    }
          
}

void start_elevator_thread(char elevator_side){
  UINT status;
  
  status =  tx_mutex_get(&mutex_uart, TX_WAIT_FOREVER);

  if (status != TX_SUCCESS)
      return;
  
  
  UARTCharPut(UART0_BASE,elevator_side);
  UARTCharPut(UART0_BASE,'r');
  UARTCharPut(UART0_BASE,'\r');
  
  
  UARTCharPut(UART0_BASE,elevator_side);
  UARTCharPut(UART0_BASE,'f');
  UARTCharPut(UART0_BASE,'\r');

  
  status =  tx_mutex_put(&mutex_uart);

  if (status != TX_SUCCESS)
      return;
}

void receive_command(char elevator_side, char* command){


  if(elevator_side=='e'){
    tx_queue_receive(&uart_left, command, 2);  
  }
  else if(elevator_side=='c'){
    tx_queue_receive(&uart_center, command, 2);  
  }
  else if(elevator_side=='d'){
    tx_queue_receive(&uart_right, command, 2);  
  }

}

void receive_intern_command(char elevator_side, char* command){

  if(elevator_side=='e'){
    tx_queue_receive(&intern_left, command, TX_WAIT_FOREVER);  
  
  }
  else if(elevator_side=='c'){
    tx_queue_receive(&intern_center, command, TX_WAIT_FOREVER);  
    
  }
  else if(elevator_side=='d'){
    tx_queue_receive(&intern_right, command, TX_WAIT_FOREVER);  
    
  }

}

void change_inside_led_status(char elevator_side, int on_off, char floor){

  UINT status =  tx_mutex_get(&mutex_uart, TX_WAIT_FOREVER);
  if (status != TX_SUCCESS)
      return;
  
  if(on_off==1){
    UARTCharPut(UART0_BASE,elevator_side);
    UARTCharPut(UART0_BASE,'L');
    UARTCharPut(UART0_BASE,floor);
    UARTCharPut(UART0_BASE,'\r');
  }
  else if(on_off==-1){
    UARTCharPut(UART0_BASE,elevator_side);
    UARTCharPut(UART0_BASE,'D');
    UARTCharPut(UART0_BASE,floor);
    UARTCharPut(UART0_BASE,'\r');
  }
  
  status =  tx_mutex_put(&mutex_uart);
  if (status != TX_SUCCESS)
      return;
}

void store_received_command(char elevator_side, char* command){
  UINT status;

  if(elevator_side=='e'){
    status = tx_queue_send(&intern_left, command, TX_WAIT_FOREVER);  
    if (status != TX_SUCCESS)
          return;
  }
  else if(elevator_side=='c'){
    status = tx_queue_send(&intern_center, command, TX_WAIT_FOREVER);  
    if (status != TX_SUCCESS)
            return;
  }
  else if(elevator_side=='d'){
    status = tx_queue_send(&intern_right, command, TX_WAIT_FOREVER);  
    if (status != TX_SUCCESS)
            return;
  }
}

void move_elevator_up(char elevator_side){
  UINT status =  tx_mutex_get(&mutex_uart, TX_WAIT_FOREVER);

  if (status != TX_SUCCESS)
      return;
  
  
  UARTCharPut(UART0_BASE,elevator_side);
  UARTCharPut(UART0_BASE,'s');
  UARTCharPut(UART0_BASE,'\r');
  
  
  status =  tx_mutex_put(&mutex_uart);

  if (status != TX_SUCCESS)
      return;
}

void move_elevator_down(char elevator_side){
  UINT status =  tx_mutex_get(&mutex_uart, TX_WAIT_FOREVER);

  if (status != TX_SUCCESS)
      return;
  
  UARTCharPut(UART0_BASE,elevator_side);
  UARTCharPut(UART0_BASE,'d');
  UARTCharPut(UART0_BASE,'\r');
  
  status =  tx_mutex_put(&mutex_uart);

  if (status != TX_SUCCESS)
      return;
}

void open_doors(char elevator_side){
  UINT status =  tx_mutex_get(&mutex_uart, TX_WAIT_FOREVER);

  if (status != TX_SUCCESS)
      return;
    
  UARTCharPut(UART0_BASE,elevator_side);
  UARTCharPut(UART0_BASE,'a');
  UARTCharPut(UART0_BASE,'\r');
  
  status =  tx_mutex_put(&mutex_uart);

  if (status != TX_SUCCESS)
      return;
}

void close_doors(char elevator_side){
  UINT status =  tx_mutex_get(&mutex_uart, TX_WAIT_FOREVER);

  if (status != TX_SUCCESS)
      return;
    
  UARTCharPut(UART0_BASE,elevator_side);
  UARTCharPut(UART0_BASE,'f');
  UARTCharPut(UART0_BASE,'\r');
  
  status =  tx_mutex_put(&mutex_uart);

  if (status != TX_SUCCESS)
      return;
}

void stop_elevator(char elevator_side){
  UINT status =  tx_mutex_get(&mutex_uart, TX_WAIT_FOREVER);

  if (status != TX_SUCCESS)
      return;
  
  UARTCharPut(UART0_BASE,elevator_side);
  UARTCharPut(UART0_BASE,'p');
  UARTCharPut(UART0_BASE,'\r');
    
  status =  tx_mutex_put(&mutex_uart);

  if (status != TX_SUCCESS)
      return;
}

void elevator_thread_function(ULONG elevator_side){
  char received_command[8];
  char floor = 'a';
  char destination_floor ='a';
  char command[8];
  int stored_commands = 0;
  int is_moving = 0;
  int executing = 0;
  
  start_elevator_thread(elevator_side);
  
  while(1){
    
    memset(received_command, 0, sizeof received_command);
   
    receive_command(elevator_side, received_command);
    
    if(received_command[1]>=48 && received_command[1]<=57)
    {
      if(received_command[2]>=48 && received_command[2]<=57){
        floor = floor_char_from_decimal_and_unit(received_command[1], received_command[2]);
      }else{
        floor = floor_char_from_decimal_and_unit('0', received_command[1]);
      }
    }


    if(received_command[1]=='E' || received_command[1]=='I'){
      if(received_command[1]=='I'){
        
        change_inside_led_status(elevator_side, 1, received_command[2]);
      }

      stored_commands ++;

      store_received_command(elevator_side, received_command);
    }

    
    if(stored_commands>0 && !executing){
      receive_intern_command(elevator_side, command);
      executing=1;
      stored_commands--;
    }

    
    if(command[1]=='E'){
      destination_floor  = floor_char_from_decimal_and_unit(command[2], command[3]);
    }
    
    else if(command[1]=='I'){
      destination_floor = command[2];
    }

    if(destination_floor>floor && !is_moving){
      
      move_elevator_up(elevator_side);

      is_moving=1;
    }
    else if(destination_floor<floor && !is_moving){
      
      move_elevator_down(elevator_side);
          
      is_moving=1;
    }

    if(destination_floor == floor && executing){
      
      stop_elevator(elevator_side);
      open_doors(elevator_side);
   
      tx_thread_sleep(1000);

      close_doors(elevator_side);
      change_inside_led_status(elevator_side, -1, destination_floor);
      
      executing = 0;
      memset(command, 0, sizeof command);
      is_moving=0;
    }
    tx_thread_sleep(1);
  }

}



void UARTInit(void){
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
  
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE,(GPIO_PIN_0|GPIO_PIN_1));
  UARTConfigSetExpClk(UART0_BASE,SystemCoreClock,(uint32_t)115200,(UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));
  UARTFIFOEnable(UART0_BASE);
}

int main(){
  IntMasterEnable(); 
  SysTickPeriodSet(2500000);  
  SysTickIntEnable();
  SysTickEnable();
  UARTInit();
  
  tx_kernel_enter();
}


// definições das threads do sistema
void  tx_application_define(void *first_unused_memory)
{

    CHAR    *pointer;


#ifdef TX_ENABLE_EVENT_TRACE
    tx_trace_enable(trace_buffer, sizeof(trace_buffer), 32);
#endif
    
    tx_byte_pool_create(&byte_pool_0, "byte pool 0", byte_pool_memory, DEMO_BYTE_POOL_SIZE);

    //mutex for writing in mutex
    tx_mutex_create(&mutex_uart, "mutex uart write", TX_NO_INHERIT);

    //Alocates thread for uart receiving chars
    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    tx_thread_create(&thread_main, "uart", receive_commands_from_user, 1,  
            pointer, DEMO_STACK_SIZE, 
            0, 0, 1, TX_AUTO_START);
            
    //Alocates threads for elevators
    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    tx_thread_create(&thread_2, "center elevator", elevator_thread_function, 'c',  
            pointer, DEMO_STACK_SIZE, 
            0, 0, 1, TX_AUTO_START);

    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    tx_thread_create(&thread_3, "right elevator", elevator_thread_function, 'd',  
            pointer, DEMO_STACK_SIZE, 
            0, 0, 1, TX_AUTO_START);

    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    tx_thread_create(&thread_1, "left elevator", elevator_thread_function, 'e',  
            pointer, DEMO_STACK_SIZE, 
            0, 0, 1, TX_AUTO_START);

    
    //intern queues for each elevator store their commands
    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&intern_left, "intern left queue", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE*sizeof(ULONG));

    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&intern_center, "intern center queue", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE*sizeof(ULONG));

    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&intern_right, "intern right queue", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE*sizeof(ULONG));

    //queues for receiving uart command
    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&uart_left, "left queue", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE*sizeof(ULONG));

    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&uart_center, "center queue", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE*sizeof(ULONG));

    tx_byte_allocate(&byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&uart_right, "right queue", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE*sizeof(ULONG));


    tx_block_release(pointer);
}
