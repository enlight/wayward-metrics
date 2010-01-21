# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""


from twisted.internet import reactor
from wayward.metrics.client import factory, protocol

metricsClient = None
_sessionID = None

class MetricsClient(protocol.MetricsClientProtocol):
    def connectionMade(self):
        global metricsClient
        metricsClient = self
        self.startSession(_sessionID)


def initialize(host, port, sessionID):
    global _sessionID
    _sessionID = sessionID
    clientFactory = factory.MetricsClientFactory()
    clientFactory.protocol = MetricsClient
    return reactor.connectTCP(host, port, clientFactory)

def recordData(buffer):
    metricsClient.recordData(buffer)

def listSessions():
    metricsClient.listSessions()
