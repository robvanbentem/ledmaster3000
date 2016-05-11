#!/usr/bin/env python3

import math, time

NUM_LEDS = 15

steps = {}
size = 2
pos = 2
maxint = 100
direction = 1
speed = 5
count = 5

leds = {}

while True:

    steps = {}
    leds = {}

    waveleds = 2 * size + 1

    prev = 0.0
    for n in range(0, waveleds+1):

        if n > size:
            cur = math.sin(math.pi * (180 / (waveleds * speed) * (n+1 - (count % speed))) / 180)
        else:
            cur = math.sin(math.pi * (180 / (waveleds * speed) * (n + (speed - (count % speed)))) / 180)

        steps[n] = cur
        print("%.2f " % cur, end="")
    print()


    #print("lpos: %d lrange:%d ldir:%d lspd:%d" % (lpos, lrange, ldir, lspd))

    offset = pos - size

    prev = 0.0
    for n in range(0, NUM_LEDS):
        
        if n > pos:
            wavepos = abs(n - (pos - size))
        else:
            wavepos = n - (pos - size)


        if wavepos < 0 or n > pos+size or wavepos >= waveleds:
            leds[n] = '_.__ '
            #print("-pos:%d wp:%d n:%d v:0.00" % (pos, wavepos, n))
            continue

        # 0 1 2 3 4 5 6 7 8 9
        #     S       S   
        #         P   N         =    
        #     N   P     


    
        if wavepos > 0:
            nx = steps[wavepos-1]
        else:
            nx = 0.0
        
        diff = abs(steps[wavepos] - nx)

        if count % speed == 0:
            btw = steps[wavepos]
        else:
            btw = nx + (diff / speed * (count % speed))

        tmp = steps[wavepos] - btw


        leds[n] = "%.2f " % steps[wavepos]
        #print("+pos:%d wp:%d n:%d v:%.2f" % (pos, wavepos, n, steps[wavepos]))



    #for p in leds.keys():
        #if direction == 1:
        #    print(leds[p], end="")
        #else:
        #    print(leds[NUM_LEDS-p-1], end="")

    #print("\n----")
    print()
    time.sleep(0.1)

    count += 1

    if count % speed == 0:
        pos = pos + 1

    if pos - size >= NUM_LEDS:
        direction = -direction
        pos = -size


    continue

    print(','.join(leds.values()))
    #break
    time.sleep(1.01)
