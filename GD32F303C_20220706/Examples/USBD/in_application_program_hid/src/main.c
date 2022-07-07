/*!
    \file    main.c
    \brief   USB IAP device

    \version 2020-08-01, V3.0.0, firmware for GD32F30x
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

#include "usb_iap_core.h"
#include "usbd_hw.h"

usb_dev usbd_iap;

/*!
    \brief      main routine
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    uint32_t app_address;
    app_func application;

    /* configure Tamper key to run firmware */
    gd_eval_key_init(KEY_TAMPER, KEY_MODE_GPIO);

    /* tamper key must be pressed on board when power on */
    if(0U !=gd_eval_key_state_get(KEY_TAMPER)) {
        /* test if user code is programmed starting from address 0x8008000 */
        if (((*(__IO uint32_t*)APP_LOADED_ADDR) & 0x2FFE0000U) == 0x20000000U) {
            app_address = *(__IO uint32_t*) (APP_LOADED_ADDR + 4U);
            application = (app_func) app_address;

            /* initialize user application's stack pointer */
            __set_MSP(*(__IO uint32_t*) APP_LOADED_ADDR);

            /* jump to user application */
            application();
        }
    }

    /* system clocks configuration */
    rcu_config();

    /* GPIO configuration */
    gpio_config();

    /* USB device configuration */
    usbd_init(&usbd_iap, &iap_desc, &iap_class);

    /* NVIC configuration */
    nvic_config();

    /* enabled USB pull-up */
    usbd_connect(&usbd_iap);

    while (1){
    }
}
