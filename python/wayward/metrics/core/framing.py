# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

import bert
import struct
gCommandHeaderStruct = struct.Struct("!II") # Python 2.5 and later, minor performance gain.

def encode(messageType, correlationID, payload):
    return gCommandHeaderStruct.pack(messageType, correlationID) + payload


def decode(buffer):
    (messageType, correlationID) = gCommandHeaderStruct.unpack(buffer[:8])
    payload = buffer[8:]
    return (messageType, correlationID, payload)


def encodePayload(*args, **kwargs):
    return bert.encode(args)


def decodePayload(payload):
    return bert.decode(payload)


