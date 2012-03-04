#!/usr/bin/env python2

# counts to 16

import serial, time

cxn = serial.Serial("/dev/ttyS0", 38400, parity=serial.PARITY_EVEN);
print ":: opened", cxn.portstr;

i = 0

decay = 0.7;

_ = [0.0]*16;
_[ 0 ] = 255.0;
idx = 0;
dir = 'f';

while True:
    cxn.write(chr(0xAA));
    cxn.write(chr(0x20));

    for __ in xrange(len(_)):
        cxn.write(chr(int(_[__])));
        _[__] *= decay;

    _[idx] = 255.0;

    if idx == 0:
        dir = 'f';
        idx = 1;
    elif idx == 15:
        dir = 'b';
        idx = 14;
    else:
        if dir == 'f':   idx += 1;
        elif dir == 'b': idx -= 1;

    cxn.flush();

    time.sleep(0.05);
