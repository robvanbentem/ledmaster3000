#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "math.h"
#include <WS2812/WS2812.h>
#include "user_interface.h"
#include "ets_sys.h"

#define LED_DEBUG 0

#define LED_PIN 14
#define LED_NUM 22
#define LED_TOTAL LED_NUM * 3

Timer procTimer;
Timer waveTimer;

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
    uint8_t repeat = 1;

    float fade_speed = 0.0f;
    float fade_position = 0;
    int8_t fade_direction = 0; // -1 is fade in, 1 is fade out
    uint8_t fade_ends = 0;
    uint8_t faded = 0;

    uint64_t life = 0;
    uint64_t max_life = 0;
    uint8_t dead = 0;
} waves[2];

char leds[LED_TOTAL];

uint32_t speed_counter = 1;

void clear_leds(char leds[], int pos) {
    memset(&leds[pos * 3], 0, sizeof(char) * 3);
}

void oc(char on) {
    if (on) {
        REG_SET_BIT(0x3ff00014, BIT(0));
        system_update_cpu_freq(160);
    } else {
        REG_SET_BIT(0x3ff00014, BIT(1));
        system_update_cpu_freq(80);
    }
}

void reset() {
    waves[0].enabled = 0;
    waves[0].pos = 10;
    waves[0].size = 2;
    waves[0].direction = 1;
    waves[0].speed = 32;
    waves[0].blue = 192;
    waves[0].repeat = 0;
    waves[0].fade_speed = 0.15f;
    waves[0].fade_direction = 1;
    waves[0].fade_position = 0;
    waves[0].life = 0;
    waves[0].max_life = 6;
    waves[0].dead = 1;
}

void run_wave() {
    oc(1);

    wave wv;
    int waveleds, wavepos, n, midmax, dirpos;
    float midstep, fade_factor;

    memset(&leds, 0, sizeof(leds));

    for (char w = 0; w < sizeof(waves) / sizeof(waves[0]); w++) {
        if (!waves[w].enabled) continue;

        waveleds = 2 * waves[w].size + 1;

        if (!waves[w].dead && !waves[w].faded) {
            if (waves[w].fade_speed != 0.0f) {
                fade_factor = (waves[w].fade_position +
                               ((float) waves[w].counter / (float) waves[w].speed) * fabs(waves[w].fade_speed)) / 1.0f;
                if (waves[w].fade_direction == -1) {
                    fade_factor = 1.0f - fade_factor;
                }
            } else {
                fade_factor = 1.0f;
            }

            for (n = 0; n < LED_NUM; n++) {
                if (n > waves[w].pos)
                    wavepos = abs(n - (waves[w].pos - waves[w].size));
                else
                    wavepos = n - (waves[w].pos - waves[w].size);

                if (wavepos < 0 || n > (waves[w].pos + waves[w].size) || wavepos >= waveleds)
                    continue;

                midmax = (waveleds * waves[w].speed);
                midstep = (wavepos * waves[w].speed) + (waves[w].speed - waves[w].counter);

                dirpos = waves[w].direction == 1 ? n * 3 : LED_TOTAL - ((n + 1) * 3);

                float factor = sin(midstep * (PI / midmax)) * fade_factor;
                leds[dirpos] += factor * waves[w].red;
                leds[dirpos + 1] += factor * waves[w].green;
                leds[dirpos + 2] += factor * waves[w].blue;
            }
        }

        waves[w].counter++;
        if (waves[w].counter == waves[w].speed) {
            waves[w].counter = 0;
            waves[w].life++;

            if (waves[w].dead) continue;

            if (waves[w].fade_speed != 0) {
                waves[w].fade_position += (float) (waves[w].fade_speed);

                if (waves[w].fade_position >= 1.0f) {
                    if (waves[w].fade_direction == -1) {
                        waves[w].faded = 1;
                    }

                    waves[w].fade_speed = 0.0f;
                    waves[w].fade_direction = 0;
                    waves[w].fade_position = 0;

                    if (waves[w].max_life && waves[w].life > waves[w].max_life) {
                        waves[w].dead = 1;
                    }
                }
            }

            if (waves[w].max_life && waves[w].life == waves[w].max_life) {
                waves[w].fade_speed = 0.1f;
                waves[w].fade_direction = -1;

                if (waves[w].fade_position != 0.0f) {
                    waves[w].fade_position = 1.0f - waves[w].fade_position;
                }
            }

            waves[w].pos = waves[w].pos + 1;

            if (waves[w].fade_ends && waves[w].fade_direction == 0) {
                if (waves[w].pos + waves[w].size == LED_NUM) {
                    waves[w].fade_speed = 1.0f / (waves[w].size * 2 - 1);
                    waves[w].fade_direction = -1;
                }
            }

            if (waves[w].pos - waves[w].size >= LED_NUM) {
                if (!waves[w].repeat) {
                    waves[w].dead = 1;
                    continue;
                }

                waves[w].direction = -waves[w].direction;
                waves[w].pos = -waves[w].size;

                if (waves[w].fade_ends && waves[w].fade_direction == 0 && waves[w].faded == 1) {
                    waves[w].fade_speed = 1.0f / (waves[w].size * 2);
                    waves[w].fade_direction = 1;
                    waves[w].faded = 0;
                }
            }
        }

    }


    oc(0);
    ws2812_writergb(LED_PIN, leds, sizeof(leds));
    oc(1);

    if (LED_DEBUG) {
        Serial.printf("\r\n");
        for (int i = 0; i < LED_NUM; i++) {
            Serial.printf("%d: %d %d %d\r\n---------\r\n", i, leds[i * 3], leds[i * 3 + 1],
                          leds[i * 3 + 2]);
        }
    }

    waveTimer.start(false);
}

void start_strip() {
    Serial.printf("Starting waveTimer\n");
    waveTimer.initializeUs(10, run_wave).start(false);
}

void init() {

    Serial.begin(921600); // 115200 by default
    Serial.systemDebugOutput(true); // Debug output to serial
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

    waves[1].enabled = 1;
    waves[1].pos = 0;
    waves[1].size = 6;
    waves[1].direction = 1;
    waves[1].speed = 75;
    waves[1].red = 64;
    waves[1].repeat = 1;
    waves[1].fade_speed = 0.1f;
    waves[1].fade_direction = 1;
    waves[1].fade_position = 0;
    waves[1].fade_ends = 1;

    reset();

/*    waves[1].enabled = 1;
      waves[1].pos = 6;
      waves[1].size = 5;
      waves[1].direction = 1;
      waves[1].speed = 1;
      waves[1].blue = 128;

      waves[2].enabled = 1;
      waves[2].pos = 192;
      waves[2].size = 128;
      waves[2].direction = 1;
      waves[2].speed = 7;
      waves[2].blue = 32;
      waves[2].red = 32;

      waves[3].enabled = 1;
      waves[3].pos = 30;
      waves[3].size = 40;
      waves[3].direction = -1;
      waves[3].speed = 3;
      waves[3].green = 64;

      waves[4].enabled = 1;
      waves[4].pos = 244;
      waves[4].size = 255;
      waves[4].direction = 1;
      waves[4].speed = 5;
      waves[4].green = 4;
      waves[4].blue = 4;

      waves[5].enabled = 1;
      waves[5].pos = 244;
      waves[5].size = 76;
      waves[5].direction = 1;
      waves[5].speed = 2;
      waves[5].blue = 32;
      waves[5].red = 32;*/

    //Serial.printf("Set CPU CLK: %u MHz\n", ets_get_cpu_frequency());

    oc(1);

    Serial.printf("Starting procTimer\n");
    procTimer.initializeMs(1000, start_strip).start(false);
}
