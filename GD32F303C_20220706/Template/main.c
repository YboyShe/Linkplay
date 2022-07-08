/*!
    \file    main.c
    \brief   led spark with systick, USART print and key example

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
    \version 2020-09-30, V2.1.0, firmware for GD32F30x 
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32f30x.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "gd32f307c_eval.h"


#define arraysize                  10
#define SET_SPI0_NSS_HIGH          gpio_bit_set(GPIOA,GPIO_PIN_8);
#define SET_SPI0_NSS_LOW           gpio_bit_reset(GPIOA,GPIO_PIN_8);
#define I2C0_OWN_ADDRESS7          0x72
#define I2C0_SLAVE_ADDRESS7        0x82

#define BOARD_ADC_CHANNEL          ADC_CHANNEL_0
#define ADC_GPIO_PORT_RCU          RCU_GPIOA
#define ADC_GPIO_PORT              GPIOA
#define ADC_GPIO_PIN               GPIO_PIN_0

uint8_t spi0_send_array[arraysize] = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA};
uint8_t spi2_send_array[arraysize] = {0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA};
uint8_t spi0_receive_array[arraysize];
uint8_t spi2_receive_array[arraysize];

uint8_t i2c_transmitter[16];

uint16_t adc_value;

void rcu_config(void);
void gpio_config(void);
void dma_config(void);
void spi_config(void);
void i2c_config(void);
void adc_config(void);
void nvic_configuration(void);
void timer_config(void);


#define ARRAY_SIZE(arr)     (sizeof(arr)/sizeof(arr[0]))
	
typedef struct _cmd_entry {
    char *command;
	uint32_t addrdata;
//    uint32_t (*function)(uint32_t);
//	volatile uint32_t * addr;
} cmd_entry;

cmd_entry cmd_table[] = {
    {"system clock is                      ", 0},
    {"AHB    clock is                      ", 0},
    {"APB1   clock is                      ", 0},
    {"APB2   clock is                      ", 0},
    {"reset source / clock register        ", 0},
    {"FMC wait state register              ", 0},
    {"FMC unlock key register 0            ", 0},
    {"FMC option bytes unlock key register ", 0},
    {"FMC status register 0                ", 0},
    {"FMC control register 0               ", 0},
    {"FMC address register 0               ", 0},
    {"FMC option bytes status register     ", 0},
    {"FMC erase/program protection register", 0},
    {"FMC unlock key register 1            ", 0},
    {"FMC status register 1                ", 0},
    {"FMC control register 1               ", 0},
    {"FMC address register 1               ", 0},
    {"FMC wait state enable register       ", 0},
    {"FMC product ID register              ", 0},
    {"option byte security protection      ", 0},
    {"option byte user                     ", 0},
    {"option byte data bit[7:0]            ", 0},
    {"option byte write protection 0       ", 0}
};

int i;
char *name;
uint32_t adders;

/*!
    \brief      toggle the led every 500ms
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_spark(void)
{
    static __IO uint32_t timingdelaylocal = 0U;

    if(timingdelaylocal){

        if(timingdelaylocal < 500U){
            gd_eval_led_on(LED4);
        }else{
            gd_eval_led_off(LED4);
        }

        timingdelaylocal--;
    }else{
        timingdelaylocal = 1000U;
    }
}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* configure systick */
    systick_config();
    /* initilize the LED, USART and key */
    gd_eval_led_init(LED4);
    gd_eval_led_init(LED3);
    gd_eval_led_init(LED5);
    gd_eval_com_init(EVAL_COM0);
    gd_eval_com_init(EVAL_COM1);
    /* configure the tamper key */
    gd_eval_key_init(KEY_WAKEUP, KEY_MODE_EXTI);
    /* peripheral clock enable */
    rcu_config();
    /* GPIO config */
    gpio_config();
    /* DMA config */
    dma_config();
    /* SPI config */
    spi_config();
	
    i2c_config();
    adc_config();
    timer_config();
	
    nvic_configuration();
    
    /* print out the clock frequency of system, AHB, APB1 and APB2 */
    printf("\r\n");
    cmd_table[ 0].addrdata = rcu_clock_freq_get(CK_SYS);
    cmd_table[ 1].addrdata = rcu_clock_freq_get(CK_AHB);
    cmd_table[ 2].addrdata = rcu_clock_freq_get(CK_APB1);
    cmd_table[ 3].addrdata = rcu_clock_freq_get(CK_APB2);
    cmd_table[ 4].addrdata = RCU_RSTSCK;
    cmd_table[ 5].addrdata = FMC_WS;
    cmd_table[ 6].addrdata = FMC_KEY0;
    cmd_table[ 7].addrdata = FMC_OBKEY;
    cmd_table[ 8].addrdata = FMC_STAT0;
    cmd_table[ 9].addrdata = FMC_CTL0;
    cmd_table[10].addrdata = FMC_ADDR0;
    cmd_table[11].addrdata = FMC_OBSTAT;
    cmd_table[12].addrdata = FMC_WP;
    cmd_table[13].addrdata = FMC_KEY1;
    cmd_table[14].addrdata = FMC_STAT1;
    cmd_table[15].addrdata = FMC_CTL1;
    cmd_table[16].addrdata = FMC_ADDR1;
    cmd_table[17].addrdata = FMC_WSEN;
    cmd_table[18].addrdata = FMC_PID;
    cmd_table[19].addrdata = OB_SPC;
    cmd_table[20].addrdata = OB_USER;
    cmd_table[21].addrdata = OB_DATA1;
    cmd_table[22].addrdata = OB_WP0;

    for (i = 0; i < ARRAY_SIZE(cmd_table); i++)
    {
        name = cmd_table[i].command;
        adders = cmd_table[i].addrdata;
        if(i < 4) printf("%s = %d\r\n", cmd_table[i].command, cmd_table[i].addrdata);
    else printf("%s = %08X\r\n", cmd_table[i].command, cmd_table[i].addrdata);
        delay_1ms(10);
    }
    rcu_all_reset_flag_clear();


    SET_SPI0_NSS_HIGH

    /* SPI enable */
    spi_enable(SPI2);
    spi_enable(SPI0);

    /* DMA channel enable */
    dma_channel_enable(DMA0, DMA_CH1);
    dma_channel_enable(DMA0, DMA_CH2);
    dma_channel_enable(DMA1, DMA_CH0);
    dma_channel_enable(DMA1, DMA_CH1);

    SET_SPI0_NSS_LOW

    /* SPI DMA enable */
    spi_dma_enable(SPI2, SPI_DMA_TRANSMIT);
    spi_dma_enable(SPI2, SPI_DMA_RECEIVE);
    spi_dma_enable(SPI0, SPI_DMA_TRANSMIT);
    spi_dma_enable(SPI0, SPI_DMA_RECEIVE);

    /* wait dma transmit complete */
    while(!dma_flag_get(DMA0,DMA_CH2, DMA_INTF_FTFIF));
    while(!dma_flag_get(DMA1,DMA_CH1, DMA_INTF_FTFIF));
    while(!dma_flag_get(DMA1,DMA_CH0, DMA_INTF_FTFIF));
    while(!dma_flag_get(DMA0,DMA_CH1, DMA_INTF_FTFIF));

    SET_SPI0_NSS_HIGH



    for(i=0; i<16; i++){
        i2c_transmitter[i]=i+0x80;
    }

    /* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, I2C0_SLAVE_ADDRESS7, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
    /* wait until the transmit data buffer is empty */
    while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));

    for(i=0; i<16; i++){
        /* data transmission */
        i2c_data_transmit(I2C0, i2c_transmitter[i]);
        /* wait until the TBE bit is set */
        while(!i2c_flag_get(I2C0, I2C_FLAG_TBE));
    }
    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C0);
    /* wait until stop condition generate */ 
    while(I2C_CTL0(I2C0)&0x0200);



    adc_software_trigger_enable(ADC1, ADC_REGULAR_CHANNEL);

    /* infinite loop */
    while (1){
        adc_flag_clear(ADC1, ADC_FLAG_EOC);
        while(SET != adc_flag_get(ADC1, ADC_FLAG_EOC)){
        }
        adc_value = ADC_RDATA(ADC1);        
        printf("ADC1: 0x%x\r\n", adc_value);
        printf("\r\n ***********************************\r\n");
        delay_1ms(500);
    }
}

/*!
    \brief      configure different peripheral clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable GPIOx clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    /* enable DMA clock */
    rcu_periph_clock_enable(RCU_DMA0);
    rcu_periph_clock_enable(RCU_DMA1);
    /* enable SPI clock */
    rcu_periph_clock_enable(RCU_SPI2);
    rcu_periph_clock_enable(RCU_SPI1);
    /* enable AF clock */
    rcu_periph_clock_enable(RCU_AF);
    /* enable I2C0 clock */
    rcu_periph_clock_enable(RCU_I2C0);
    /* enable ADC1 clock */
    rcu_periph_clock_enable(RCU_ADC1);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV4);
}

/*!
    \brief      configure the GPIO peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void gpio_config(void)
{
    /* SPI0 GPIO config: SCK/PA5, MISO/PA6, MOSI/PA7 */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_7);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    /* PA3 as NSS */
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);

    gpio_pin_remap_config(GPIO_SPI2_REMAP, ENABLE);
    /* SPI1 GPIO config: NSS/PA15, SCK/PB3, MISO/PB4, MOSI/PB5 */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    /* connect PB6 to I2C0_SCL */
    /* connect PB7 to I2C0_SDA */
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
    /* config the GPIO as analog mode */
    gpio_init(ADC_GPIO_PORT, GPIO_MODE_AIN, GPIO_OSPEED_MAX, ADC_GPIO_PIN);
	
	/* LED GPIO config */
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
}

/*!
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void dma_config(void)
{
    dma_parameter_struct dma_init_struct;
    
    /* SPI0 transmit dma config:DMA0-DMA_CH2  */
    dma_deinit(DMA0, DMA_CH2);
    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI0);
    dma_init_struct.memory_addr  = (uint32_t)spi0_send_array;
    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority     = DMA_PRIORITY_LOW;
    dma_init_struct.number       = arraysize;
    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(DMA0, DMA_CH2, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH2);
    dma_memory_to_memory_disable(DMA0, DMA_CH2);

    /* SPI0 receive dma config:DMA0-DMA_CH1  */
    dma_deinit(DMA0, DMA_CH1);
    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI0);
    dma_init_struct.memory_addr  = (uint32_t)spi0_receive_array;
    dma_init_struct.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.priority     = DMA_PRIORITY_HIGH;
    dma_init(DMA0, DMA_CH1, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH1);
    dma_memory_to_memory_disable(DMA0, DMA_CH1);

    /* SPI2 transmit dma config:DMA1,DMA_CH1  */
    dma_deinit(DMA1, DMA_CH1);
    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI2);
    dma_init_struct.memory_addr  = (uint32_t)spi2_send_array;
    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.priority     = DMA_PRIORITY_MEDIUM;
    dma_init(DMA1, DMA_CH1, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA1, DMA_CH4);
    dma_memory_to_memory_disable(DMA1, DMA_CH4);

    /* SPI2 receive dma config:DMA1,DMA_CH0  */
    dma_deinit(DMA1, DMA_CH0);
    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI2);
    dma_init_struct.memory_addr  = (uint32_t)spi2_receive_array;
    dma_init_struct.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA1, DMA_CH0, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA1, DMA_CH0);
    dma_memory_to_memory_disable(DMA1, DMA_CH0);
}

/*!
    \brief      configure the SPI peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void spi_config(void)
{
    spi_parameter_struct spi_init_struct;

    /* SPI0 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_256;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);

    /* SPI2 parameter config */
    spi_init_struct.device_mode = SPI_SLAVE;
    spi_init_struct.nss         = SPI_NSS_HARD;
    spi_init(SPI1, &spi_init_struct);
}

/*!
    \brief      cofigure the I2C0 interfaces
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_config(void)
{
    /* I2C clock configure */
    i2c_clock_config(I2C0, 100000, I2C_DTCY_2);
    /* I2C address configure */
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, I2C0_OWN_ADDRESS7);
    /* enable I2C0 */
    i2c_enable(I2C0);
    /* enable acknowledge */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
}

/*!
    \brief      configure the ADC peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void adc_config(void)
{
    /* ADC continuous function enable */
    adc_special_function_config(ADC1, ADC_CONTINUOUS_MODE, ENABLE);
    adc_special_function_config(ADC1, ADC_SCAN_MODE, DISABLE);    
    /* ADC trigger config */
    adc_external_trigger_source_config(ADC1, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE); 
    /* ADC data alignment config */
    adc_data_alignment_config(ADC1, ADC_DATAALIGN_RIGHT);
    /* ADC mode config */
    adc_mode_config(ADC_MODE_FREE); 
    /* ADC channel length config */
    adc_channel_length_config(ADC1, ADC_REGULAR_CHANNEL, 1);

    /* ADC regular channel config */
    adc_regular_channel_config(ADC1, 0, BOARD_ADC_CHANNEL, ADC_SAMPLETIME_55POINT5);
    adc_external_trigger_config(ADC1, ADC_REGULAR_CHANNEL, ENABLE);
  
    /* ADC resolusion 6B */
    adc_resolution_config(ADC1, ADC_RESOLUTION_6B);  
    
    /* enable ADC interface */
    adc_enable(ADC1);
    delay_1ms(1);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC1);
}

/*!
    \brief      configure the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
void nvic_configuration(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    nvic_irq_enable(TIMER2_IRQn, 1, 1);
}

/**
    \brief      configure the TIMER peripheral
    \param[in]  none
    \param[out] none
    \retval     none
  */
void timer_config(void)
{
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER2);
    timer_deinit(TIMER2);

    /* TIMER2 configuration */
    timer_initpara.prescaler         = 23;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 500;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER2,&timer_initpara);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER2);
    /* channel 0 interrupt enable */
    timer_interrupt_enable(TIMER2,TIMER_INT_UP);

    /* TIMER2 counter enable */
    timer_enable(TIMER2);
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM1, (uint8_t)ch);
    while(RESET == usart_flag_get(EVAL_COM1, USART_FLAG_TBE));

    return ch;
}
