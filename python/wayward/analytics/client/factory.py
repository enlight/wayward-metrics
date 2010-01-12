# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

from twisted.internet import protocol
from twisted.python import log

from wayward.analytics.client import protocol as analytics_protocol

class AnalyticsClientFactory(protocol.ClientFactory):
    protocol = analytics_protocol.AnalyticsClientProtocol


    def clientConnectionFailed(self, connector, reason):
        log.msg("Connection to WA server failed: %s" % reason)


    def clientConnectionLost(self, connector, reason):
        log.msg("Connection to WA server lost: %s" % reason)


