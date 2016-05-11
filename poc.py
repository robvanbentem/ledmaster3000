#!/usr/bin/env python3

import math, time

NUM_LEDS = 16

steps = {}
size = 4
pos = -4
maxint = 100
direction = 1
speed = 1
count = 0

leds = {}

while True:

    leds = {}

    waveleds = 2 * size + 1

    for n in range(0, NUM_LEDS):
        
        if n > pos:
            wavepos = abs(n - (pos - size))
        else:
            wavepos = n - (pos - size)

        if wavepos < 0 or n > pos+size or wavepos >= waveleds:
            leds[n] = '_.__ '
            continue

        leds[n] = "%.2f " % math.sin(math.pi * (180 / (waveleds * speed) * ((wavepos * speed) + (speed - count))) / 180)


    for p in leds.keys():
        if direction == 1:
            print(leds[p], end="")
        else:
            print(leds[NUM_LEDS-p-1], end="")

    print()
    
    time.sleep(0.1)

    count += 1

    if count == speed:
        pos = pos + 1
        count = 0

    if pos - size >= NUM_LEDS:
        direction = -direction
        pos = -size
