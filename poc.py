#!/usr/bin/env python3

import math, time

nleds = 40

leds = {}
lrange = 10
lpos = 0
lint = 10
ldir = 1
lspd = 1

while True:

    leds = {}

    affleds = 2*lrange+1
    #print("lpos: %d lrange:%d ldir:%d lspd:%d" % (lpos, lrange, ldir, lspd))


    for n in range(0, nleds):
        if n >= lpos-lrange and n <= lpos+lrange:

            cpos = n - (lpos - lrange)
            
            val = str(int(math.sin(math.pi * (180/affleds*cpos) / 180) * lint))


            dbg = "n:%d cpos:%d aff:%d val:%s" % (n, cpos, affleds, val)
            #print(dbg)

            leds[n] = str(val)
        else:
            leds[n] = "0"

    if lpos + lrange < 0 or nleds < lpos - lrange:
        ldir = -ldir

    lpos = lpos + (ldir * lspd)

    print(','.join(leds.values()))
    #break
    time.sleep(0.01)
