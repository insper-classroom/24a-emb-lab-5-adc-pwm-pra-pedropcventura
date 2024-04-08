/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include <math.h>
#include <stdlib.h>

QueueHandle_t xQueueAdc;

typedef struct adc {
    int axis;
    int val;
} adc_t;

void uart_task(void *p) {
    adc_t data;
    int msb;
    int lsb;
    int v;
    while (1) {
        if (xQueueReceive(xQueueAdc, &data, portMAX_DELAY) == pdTRUE) {
            v = data.val/1000;
            msb = data.val >> 8;
            lsb = data.val & 0xFF;
            uart_putc_raw(uart0, data.axis);
            uart_putc_raw(uart0, msb);
            uart_putc_raw(uart0, lsb);
            uart_putc_raw(uart0, -1);
            //printf("valor: %d\n", data.val);
        }
    }
}


void x_task(void *p) {
    adc_init();
    adc_gpio_init(26); 
    adc_t data;

    
    while (1) {
        adc_select_input(0); 
        data.axis = 0;
        int nt;
        nt = adc_read();
        nt -= 2048;
        nt = nt/8;
        if (nt>-30&&nt<30){
            nt = 0;
        }
        data.val = nt;

        xQueueSend(xQueueAdc, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void y_task(void *p) {
    adc_init();
    adc_gpio_init(27); 
    adc_t data;

    
    while (1) {
        adc_select_input(1); 
        data.axis = 1;
        int nt;
        nt = adc_read();
        nt -= 2048;
        nt = 255*nt/2047;
        if (nt>-30&&nt<30){
            nt = 0;
        }
        data.val = nt;

        xQueueSend(xQueueAdc, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}


int main() {
    stdio_init_all();
    adc_init();

    xQueueAdc = xQueueCreate(32, sizeof(adc_t));

    xTaskCreate(x_task, "XAxisTask", 4096, NULL, 1, NULL);
    xTaskCreate(y_task, "Y Axis Task", 4096, NULL, 1, NULL);
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
