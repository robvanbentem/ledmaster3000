#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "math.h"
#include <WS2812/WS2812.h>
#include "user_interface.h"
#include "ets_sys.h"

#define LED_DEBUG 0

#define LED_PIN 14
#define LED_NUM 16
#define LED_TOTAL LED_NUM * 3

Timer procTimer;


struct wave {
    uint8_t enabled = 0;
    int pos; // current wave center position
    uint8_t size; // size of the wave (real size = 2*size+1)
    int8_t direction;
    uint16_t speed;
    uint8_t red = 0; // max brightness of the wave
    uint8_t green = 0; // max brightness of the wave
    uint8_t blue = 0; // max brightness of the wave
    uint32_t counter = 0;
} waves[10];

char leds[LED_TOTAL];

uint32_t speed_counter = 1;

void clear_leds(char leds[], int pos) {
    memset(&leds[pos * 3], 0, sizeof(char) * 3);
}

void run_wave() {
    REG_SET_BIT(0x3ff00014, BIT(0));
    system_update_cpu_freq(160);

    wave dawave;
    int waveleds, wavepos, n, min, max;
    float intensity, curint, nexint;

    memset(&leds, 0, sizeof(leds));

    for (char w = 0; w < sizeof(waves) / sizeof(waves[0]); w++) {
        if (!waves[w].enabled) continue;

        waveleds = 2 * waves[w].size + 1;
        max = waves[w].pos + waves[w].size >= LED_NUM - 1 ? LED_NUM - 1 : waves[w].pos + waves[w].size;
        min = waves[w].pos - waves[w].size < 0 ? 0 : waves[w].pos - waves[w].size;

        float steps[waveleds];
        for (n = 0; n < waveleds; n++) {
            steps[n] = sin(n * (PI / waveleds));
        }

        for (n = min; n < max; n++) {
            //if (n >= waves[w].pos - waves[w].size && n <= waves[w].pos + waves[w].size) {
            wavepos = n - (waves[w].pos - waves[w].size);

            if (waves[w].counter % waves[w].speed == 0) {

                intensity = sin(wavepos * (PI / waveleds));

                if (waves[w].green) leds[n * 3 + 0] += intensity * waves[w].green;
                if (waves[w].red) leds[n * 3 + 1] += intensity * waves[w].red;
                if (waves[w].blue) leds[n * 3 + 2] += intensity * waves[w].blue;
            } else {

                if (n == min || n == max - 1) {
                    curint = steps[wavepos];
                    if (n == min && waves[w].direction) {
                        nexint = steps[wavepos + 1];
                    } else if (n == max - 1 && waves[w].direction == -1) {
                        nexint = steps[wavepos - 1];
                    } else {
                        nexint = 0.0;
                    }
                } else {
                    curint = steps[wavepos];
                    nexint = steps[wavepos + waves[w].direction];
                }

                if (nexint > curint) {
                    intensity =
                            curint +
                            (nexint - curint) /
                            waves[w].speed *
                            (waves[w].speed - (waves[w].counter % waves[w].speed));
                } else {
                    intensity =
                            curint -
                            (curint - nexint) /
                            waves[w].speed *
                            (waves[w].counter % waves[w].speed);
                }

                if (waves[w].red) {
                    leds[n * 3 + 1] += intensity * waves[w].red;
                }

                if (waves[w].green) {
                    leds[n * 3] += intensity * waves[w].green;
                }
            }
            //}



        }


        if (waves[w].counter % waves[w].speed == 0) {
            if (waves[w].pos + waves[w].size < 0 || LED_NUM < waves[w].pos - waves[w].size) {
                waves[w].direction = -waves[w].direction;
            }
            waves[w].pos += (waves[w].direction); // * waves[w].speed);
        }

        waves[w].counter++;
    }


    REG_SET_BIT(0x3ff00014, BIT(1));
    system_update_cpu_freq(80);
    ws2812_writegrb(LED_PIN, leds, sizeof(leds));

    if (LED_DEBUG) {
        for (int i = 0; i < LED_NUM; i++) {
            Serial.printf("%d: %d %d %d\r\n---------\r\n", i, leds[i * 3], leds[i * 3 + 1],
                          leds[i * 3 + 2]);
        }
    }

    procTimer.start(false);
}

void init() {

    Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
    Serial.systemDebugOutput(false); // Debug output to serial
    WifiStation.enable(false, true);

    pinMode(0, INPUT);
    digitalWrite(0, 1);
    pinMode(2, INPUT);
    digitalWrite(2, 1);

    pinMode(15, INPUT);
    digitalWrite(15, 0);
    pinMode(12, OUTPUT);
    digitalWrite(12, 0);
    pinMode(13, OUTPUT);
    digitalWrite(13, 0);

    waves[0].enabled = 0;
    waves[0].pos = 0;
    waves[0].size = 2;
    waves[0].direction = 1;
    waves[0].speed = 20;
    waves[0].red = 8;

    waves[1].enabled = 1;
    waves[1].pos = 0;
    waves[1].size = 3;
    waves[1].direction = 1;
    waves[1].speed = 10;
    waves[1].green = 16;

    waves[2].enabled = 0;
    waves[2].pos = 0;
    waves[2].size = 5;
    waves[2].direction = 1;
    waves[2].speed = 100;
    waves[2].red = 16;
    /*

    waves[3].enabled = 1;
    waves[3].pos = 12;
    waves[3].size = 67;
    waves[3].direction = -1;
    waves[3].speed = 1;
    waves[3].blue = 8;
    */
    //Serial.printf("Set CPU CLK: %u MHz\n", ets_get_cpu_frequency());

    REG_SET_BIT(0x3ff00014, BIT(0));
    system_update_cpu_freq(160);

    procTimer.initializeMs(100, run_wave).start(false);
}
