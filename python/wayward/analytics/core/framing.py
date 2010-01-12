# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

import bert
import struct
gLengthPrefixStruct = struct.Struct("!I") # Python 2.5 and later, minor performance gain.

def encode(messageType, payload):
    return gLengthPrefixStruct.pack(messageType) + payload


def decode(buffer):
    (messageType,) = gLengthPrefixStruct.unpack(buffer[:4])
    payload = buffer[4:]
    return (messageType, payload)


def encodePayload(*args, **kwargs):
    return bert.encode(args)


def decodePayload(payload):
    return bert.decode(payload)


