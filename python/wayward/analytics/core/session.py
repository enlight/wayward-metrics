# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

import os
import struct
from twisted.python import log

gLengthPrefixStruct = struct.Struct("!I") # Python 2.5 and later, minor performance gain.

class Session(object):
    def __init__(self, sessionID, loadFromDisk=False):
        self.sessionID = sessionID
        self.bufferLog = []
        baseFileName = os.path.join('sessions', sessionID)
        if loadFromDisk:
            # XXX: Existing session ... pull the data about it from disk.
            self.dataFile = open(baseFileName + '.dat', 'a')
        else:
            self.dataFile = open(baseFileName + '.dat', 'a')


    def recordData(self, buffer):
        self.bufferLog.append(buffer)
        if len(self.bufferLog) > 5000:
            self.archiveData()


    def archiveData(self):
        log.msg("Archiving data...")
        diskBuffer = ''
        for b in self.bufferLog:
            diskBuffer += (gLengthPrefixStruct.pack(len(b)) + b + '\n')
        self.dataFile.write(diskBuffer)
        self.bufferLog = []
        log.msg("Done.")


    def close(self):
        self.archiveData()
        self.dataFile.close()
        self.dataFile = None


