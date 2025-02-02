/*
 * Copyright (C) 2017 iCub Facility - Istituto Italiano di Tecnologia
 * Author:  Marco Accame
 * email:   marco.accame@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/


// --------------------------------------------------------------------------------------------------------------------
// - description of application
//
// this is a very simple application which uses stm32 hal as generated by cube-mx but encapsulated in a .lib file.
// you have to do everything by yourself in here by calling what stm gives you. 
// my personal recommendation is to use the appslim version which uses the embot c++ objects 



#define APPSLIM_STM32HAL_TICK_SYSTICK
#undef APPSLIM_DO_NOTHING


#if     defined(APPSLIM_STM32HAL_TICK_NONE)
    #warning  APPSLIM_STM32HAL_TICK_NONE is defined: this application will work, but some funtions of stm32hal which use HAL_GetTick() will not work properly
#elif   defined(APPSLIM_STM32HAL_TICK_SYSTICK)
    #warning  APPSLIM_STM32HAL_TICK_SYSTICK is defined: we start the systick at 1 ms
#else    
    #error: choose a APPSLIM_STM32HAL_TICK_* mode amongst: [APPSLIM_STM32HAL_TICK_NONE, APPSLIM_STM32HAL_TICK_SYSTICK] 
#endif  


// - includes

#include "stm32hal.h" 
  

#if     defined(APPSLIM_STM32HAL_TICK_SYSTICK)
    
static volatile uint64_t s_1mstickcount = 0;
    
void SysTick_Handler(void)
{
    s_1mstickcount++;
}

static void tick1msecinit(void)
{
    HAL_SYSTICK_Config(SystemCoreClock/1000);
}
    
static uint32_t tick1msecget(void)
{
    return (uint32_t)s_1mstickcount;
}

#else

static void tick1msecinit(void)
{   
}

static uint32_t tick1msecget(void)
{   // one must always provide one function which counts forward
    static uint32_t n = 0;
    return n++;
}

#endif

  

int main(void)
{ 
    // init the stm32hal: 
    // give one funtion which init the 1 ms tick, 
    // give one funtion which returns the number of ticks, 
    stm32hal_config_t cfg = {0};
    cfg.tick1ms_init = tick1msecinit;
    cfg.tick1ms_get = tick1msecget;
        
    stm32hal_init(&cfg);    
    
#if     defined(APPSLIM_DO_NOTHING)
    
    // description: do nothing but executing a dummy loop
    
    for(;;);   
    
#else
    
    // description: 
    // led zero (red) always pulses at 1 hz             

    // init gpio and turn off
    // init gpio: already done inside stm32hal_init()
    // turn off
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    

    // turn it on
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
    
    uint32_t flagON = 1;
    
    for(;;)
    {
        
        // delay of 500 ms: it is ok only if the 1 ms tick is correctly initted         
        HAL_Delay(500);
        
        if(1 == flagON)
        {
            flagON = 0;
            // turn off
            HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);            
        }
        else
        {
            flagON = 1;
            // turn on
            HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);                   
        }

                    
    }
    
#endif
    
}



// - end-of-file (leave a blank line after)----------------------------------------------------------------------------




