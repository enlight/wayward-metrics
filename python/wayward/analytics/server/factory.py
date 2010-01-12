# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

from twisted.internet import protocol
from twisted.python import log

from wayward.analytics.core import sessionstore
from wayward.analytics.server import protocol as analytics_protocol

class AnalyticsServerFactory(protocol.ServerFactory):
    """
    Server factory and tracker for L{AnalyticsServerProtocol} connections.  This
    class also provides a session store for tracking available session data.

    @ivar connections: A list of all the connected L{AnalyticsServerProtocol}
        instances using this object as their controller.
    @type connections: C{list} of L{AnalyticsServerProtocol}
    @ivar sessionStore: A session storage manager.
    @type sessionStore: L{SessionStore}
    """

    protocol = analytics_protocol.AnalyticsServerProtocol

    def __init__(self):
        self.connections = []
        self.sessionStore = sessionstore.SessionStore()


    def buildProtocol(self, addr):
        p = self.protocol(self)
        return p


    def connectionMade(self, protocol):
        """
        Track a newly connected L{AnalyticsServerProtocol}.
        """
        self.connections.append(protocol)


    def connectionLost(self, protocol):
        """
        Stop tracking a no-longer connected L{AnalyticsServerProtocol}.
        """
        self.connections.remove(protocol)


