# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""


from twisted.internet import reactor
from wayward.analytics.client import factory, protocol

analyticsClient = None
_sessionID = None

class AnalyticsClient(protocol.AnalyticsClientProtocol):
    def connectionMade(self):
        global analyticsClient
        analyticsClient = self
        self.startSession(_sessionID)


def initialize(host, port, sessionID):
    global _sessionID
    _sessionID = sessionID
    clientFactory = factory.AnalyticsClientFactory()
    clientFactory.protocol = AnalyticsClient
    return reactor.connectTCP(host, port, clientFactory)

def recordData(buffer):
    analyticsClient.recordData(buffer)

def listSessions():
    analyticsClient.listSessions()
