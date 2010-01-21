# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

from twisted.internet import protocol
from twisted.python import log

from wayward.metrics.core import sessionstore
from wayward.metrics.server import protocol as metrics_protocol

class MetricsServerFactory(protocol.ServerFactory):
    """
    Server factory and tracker for L{MetricsServerProtocol} connections.  This
    class also provides a session store for tracking available session data.

    @ivar connections: A list of all the connected L{MetricsServerProtocol}
        instances using this object as their controller.
    @type connections: C{list} of L{MetricsServerProtocol}
    @ivar sessionStore: A session storage manager.
    @type sessionStore: L{SessionStore}
    """

    protocol = metrics_protocol.MetricsServerProtocol

    def __init__(self):
        self.connections = []
        self.sessionStore = sessionstore.SessionStore()


    def buildProtocol(self, addr):
        p = self.protocol(self)
        return p


    def connectionMade(self, protocol):
        """
        Track a newly connected L{MetricsServerProtocol}.
        """
        self.connections.append(protocol)


    def connectionLost(self, protocol):
        """
        Stop tracking a no-longer connected L{MetricsServerProtocol}.
        """
        self.connections.remove(protocol)


