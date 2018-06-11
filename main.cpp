#define A_TO_B

#include <stdint.h>
#include <stddef.h>
#include "mbed.h"
#include "math.h"


#define PI 3.14159265

#define PITCH       (440)
#define GAIN        (0.25)
#define COUNT       (SAI_DEFAULT_SAMPLE_RATE / PITCH)

InterruptIn     btn(BUTTON1);
DigitalOut      led1(LED1);

#ifdef A_TO_B
SAIReceiver     mic(SAI_A_MCLK, SAI_A_BCLK, SAI_A_WCLK, SAI_A_SD, &sai_mode_i2s16w32);
SAITransmitter  sp (SAI_B_MCLK, SAI_B_BCLK, SAI_B_WCLK, SAI_B_SD, &sai_mode_i2s16w32);
#else
SAIReceiver     mic(SAI_B_MCLK, SAI_B_BCLK, SAI_B_WCLK, SAI_B_SD, &sai_mode_i2s16w32);
SAITransmitter  sp (SAI_A_MCLK, SAI_A_BCLK, SAI_A_WCLK, SAI_A_SD, &sai_mode_i2s16w32);
#endif

bool g_echo = true;

void switch_mode(void) {
    g_echo = !g_echo;
    led1 = !g_echo; // update led : active low
}

int main() {
    btn.rise(&switch_mode);
    led1 = !g_echo;

    sai_result_t sp_status  = sp.status();
    sai_result_t mic_status = mic.status();

    if (sp_status != SAI_RESULT_OK || mic_status != SAI_RESULT_OK) {
        printf("Speaker SAI line status: %d\r\n", sp_status);
        printf("Mic SAI line status: %d\r\n", mic_status);
        
        error("SAI module failed to initialize properly...\r\n");
    }
    
    uint32_t i = 0;
    
    int16_t buffer[COUNT] = {0};
    printf("Count %u\r\n", COUNT);
    for (uint32_t i = 0; i < COUNT; i++) {
        double sample = sin((double)i * 2. * PI/(double)COUNT) * GAIN;
        buffer[i] = sample * INT16_MAX;
        //printf("%2lu %6hd %9.6lf %04hx\r\n", i, buffer[i], sample, buffer[i]);
    }
    
    printf("Starting loop...\r\n");
    i = 0;
    uint32_t sample = 0;
    if (g_echo) {
        sp.send(1);
    }
    while (true) {
        if (g_echo) {
            uint32_t sample = 0;
            if (mic.receive(&sample)) {
                // add 1 create a little variation we can see something when using loopback
                while (!sp.send(sample + 1)) ;
            }
        } else {
            i += 1;
            if (i == COUNT) {
                i = 0;
            }
            while (!sp.send(buffer[i])) ;
            while (!sp.send(buffer[i])) ;
        }
    }
}

