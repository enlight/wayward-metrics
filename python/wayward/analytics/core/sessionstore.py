# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

import os
from wayward.analytics.core import session

class SessionStore(object):
    def __init__(self):
        self.sessions = {}
        if not os.path.exists('sessions'):
            os.mkdir('sessions')


    def createNewSession(self, sessionID):
        s = session.Session(sessionID, loadFromDisk=False)
        self.sessions[sessionID] = s
        return s


    def getSessionByID(self, sessionID):
        if sessionID in self.sessions:
            return self.sessions[sessionID]
        return self.createNewSession(sessionID)


    def getSessions(self, **flags):
        sessionList = []
        for session in self.sessions.itervalues():
            # In the future, filter here based on flags.
            sessionList.append(session)
        return sessionList


