# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

from twisted.internet import protocol
from twisted.python import log

from wayward.metrics.client import protocol as metrics_protocol

class MetricsClientFactory(protocol.ClientFactory):
    protocol = metrics_protocol.MetricsClientProtocol


    def clientConnectionFailed(self, connector, reason):
        log.msg("Connection to WA server failed: %s" % reason)


    def clientConnectionLost(self, connector, reason):
        log.msg("Connection to WA server lost: %s" % reason)


