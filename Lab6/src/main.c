#include "tx_api.h"
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

#define DEMO_BYTE_POOL_SIZE     9120
#define DEMO_STACK_SIZE         1024

TX_THREAD               thread_0;
TX_THREAD               thread_1;
TX_THREAD               thread_2;
TX_BYTE_POOL            bytePool;
TX_MUTEX                mutex_0;

UCHAR                   byte_pool_memory[DEMO_BYTE_POOL_SIZE];


void turnOff_leds(){
  // apaga todos os leds da placa
  GPIOPinWrite(GPIO_PORTN_BASE,(GPIO_PIN_0 | GPIO_PIN_1), 
               (GPIO_PIN_0 | GPIO_PIN_1) & (0));
  GPIOPinWrite(GPIO_PORTF_BASE,(GPIO_PIN_0 | GPIO_PIN_4), 
               (GPIO_PIN_0 & 0) | (GPIO_PIN_4 & (0)));
}

void turnOn_leds(uint8_t led){
  // acende o led referenciado
  GPIOPinWrite(GPIO_PORTN_BASE,(GPIO_PIN_0 | GPIO_PIN_1), 
               (GPIO_PIN_0 | GPIO_PIN_1) & (led >> 2));
  GPIOPinWrite(GPIO_PORTF_BASE,(GPIO_PIN_0 | GPIO_PIN_4), 
               (GPIO_PIN_0 & led) | (GPIO_PIN_4 & (led << 3)));
}

uint8_t escalonamento(){
  // Escalonamento por time-slice de 50 ms. Todas as tarefas com mesma prioridade.
  //return 0;
  
  // Escalonamento sem time-slice e sem preempção.
  // return 1;

  // Escalonamento preemptivo por prioridade.
  // return 2;

  // Implemente um Mutex compartilhado entre T1 e T3.
  // return 3;

  // Implemente um Mutex compartilhado entre T1 e T3 + mutex com heranca de prioridade;
  return 4;
}

uint8_t loopDelay_led(uint8_t led, uint32_t delay_time){

  uint32_t delay_count = 0;
  
  // loop para o delay
  while(delay_count < delay_time){
    turnOn_leds(led);
    delay_count += 1;
    turnOff_leds();
  }
  return 0;
}

void esc_3_4(uint8_t led, uint8_t time){
  // função para quando o escalonamento retorna 3 ou 4

  UINT status = tx_mutex_get(&mutex_0,TX_WAIT_FOREVER);
          
  if (status != TX_SUCCESS){
    return;
  }
      
  loopDelay_led(led, time);
  tx_thread_sleep(500); // ms

  status = tx_mutex_put(&mutex_0);
    
  if (status != TX_SUCCESS){
    return;
  }
}

void args_th(ULONG th_input){
  
  if (th_input == 1){
    while(true){
      
      if(escalonamento() == 3 | escalonamento() == 4){
        // cada loop demorou 1.39 us, então para a th_1 rodar por 300 ms, 
        // preciso de 215827 loops
        esc_3_4(8, 215827);
      }
      else{
        loopDelay_led(8, 215827);
        tx_thread_sleep(500);
      }
    }
  }
  else if(th_input == 2){
    while(true){
      // cada loop demorou 1.39 us, então para a th_1 rodar por 500 ms, 
      // preciso de 359712 loops
      loopDelay_led(4, 359712);
      tx_thread_sleep(750);
        
    }
  }
  
  // muda da th_input 1 pelo led e tempo
  else if(th_input == 3){
    while(1){
      
      if(escalonamento() == 3 | escalonamento() == 4){  
        // cada loop demorou 1.39 us, então para a th_1 rodar por 300 ms, 
        // preciso de 575539 loops
        esc_3_4(2, 575539);
      }
      else{
        loopDelay_led(2, 575539);
        tx_thread_sleep(2000);
      }
    }
  }
}

void tx_application_define(void *first_unused_memory){

  CHAR    *pointer = TX_NULL;

  #ifdef TX_ENABLE_EVENT_TRACE
      tx_trace_enable(trace_buffer, sizeof(trace_buffer), 32);
  #endif
        
  UINT th_time_div[3];
  UINT th_priorities[3];
  UINT th_preemp[3];

  for(int i=0; i<3; i++){
    
    if(escalonamento()== 0){
      th_priorities[i]=0;
      th_preemp[i]=0;
      th_time_div[i]=25;
    }
    else{
      th_priorities[i]=i;
      th_preemp[i]=i;
      th_time_div[i] = TX_NO_TIME_SLICE;
      
      if(escalonamento() == 1){
        for(int i=0; i<3; i++){
          th_preemp[i]=0;
        }
      }
      else if(escalonamento()==3){
        tx_mutex_create(&mutex_0, "mutex 0", TX_NO_INHERIT);
      }
      else if(escalonamento()==4){
        tx_mutex_create(&mutex_0, "mutex 0", TX_INHERIT);
        }
      }
    }
    
    // Cria byte memory pool para alocar as threads
    tx_byte_pool_create(&bytePool, "byte pool 0",
                        byte_pool_memory, DEMO_BYTE_POOL_SIZE);

    // aloca memória para a th_1
    tx_byte_allocate(&bytePool, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    // cria th_1
    tx_thread_create(&thread_0, "th_1", args_th, 1,  
            pointer, DEMO_STACK_SIZE, 
            th_priorities[0], th_preemp[0], th_time_div[0], TX_AUTO_START);

    // aloca memória para a th 2
    tx_byte_allocate(&bytePool, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    // cria th_2
    tx_thread_create(&thread_1, "th_2", args_th, 2,  
            pointer, DEMO_STACK_SIZE, 
            th_priorities[1], th_preemp[1], th_time_div[1], TX_AUTO_START);
    
    // aloca memória para a th 3
    tx_byte_allocate(&bytePool, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    // cria th_3
    tx_thread_create(&thread_2, "th_3", args_th, 3,  
            pointer, DEMO_STACK_SIZE, 
            th_priorities[2], th_preemp[2], th_time_div[2], TX_AUTO_START);
}


int main(){
   
  // enablers
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

  // gpios config
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 );
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4 );
  
  turnOff_leds();
  
  // en threads
  tx_kernel_enter();
}