# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

from twisted.protocols import basic
from twisted.python import log

from wayward.metrics.core import constants, framing, protocol

class MetricsClientProtocol(protocol.MetricsProtocolBase):
    PROTOCOL_VERSION = 1
    dispatchTable = {
        constants.METHOD_RECEIVE_RESULT:             'metricsReceiveResult',
        constants.METHOD_RECEIVE_SESSION_LIST_COUNT: 'metricsReceiveSessionListCount',
        constants.METHOD_RECEIVE_SESSION_LIST_ITEM:  'metricsReceiveSessionListItem',
    }

    def __init__(self):
        protocol.MetricsProtocolBase.__init__(self)

    def setAuthorizationCredentials(self, username, password):
        self.sendCommand(constants.METHOD_SET_AUTHORIZATION_CREDENTIALS, 0, framing.encodePayload(username, password))


    def startSession(self, sessionID):
        self.sendCommand(constants.METHOD_START_SESSION, 0, framing.encodePayload(sessionID))


    def setProtocolVersion(self, versionNumber):
        pass


    def recordData(self, buffer):
        self.sendCommand(constants.METHOD_RECORD_DATA, 0, buffer)


    def observeSession(self, sessionID):
        self.sendCommand(constants.METHOD_OBSERVE_SESSION, 0, framing.encodePayload(sessionID))


    def stopObservingSession(self, sessionID):
        self.sendCommand(constants.METHOD_STOP_OBSERVING_SESSION, 0, framing.encodePayload(sessionID))


    def retrieveSession(self, sessionID):
        self.sendCommand(constants.METHOD_RETRIEVE_SESSION, 0, framing.encodePayload(sessionID))


    def deleteSession(self, sessionID):
        self.sendCommand(constants.METHOD_DELETE_SESSION, 0, framing.encodePayload(sessionID))


    def listSessions(self, **flags):
        self.sendCommand(constants.METHOD_LIST_SESSIONS, 0, framing.encodePayload(flags))


    def metricsReceiveSessionListCount(self, correlationID, payload):
        log.msg("Expecting %s sessions" % framing.decodePayload(payload)[0])


    def metricsReceiveSessionListItem(self, correlationID, payload):
        log.msg("Received session list item: %s" % framing.decodePayload(payload)[0])


