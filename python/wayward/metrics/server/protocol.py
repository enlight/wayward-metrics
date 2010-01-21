# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

from twisted.python import log
from wayward.metrics.core import constants, framing, protocol

class MetricsServerProtocol(protocol.MetricsProtocolBase):
    dispatchTable = {
        constants.METHOD_RECEIVE_RESULT:         'metricsReceiveResult',
        constants.METHOD_START_SESSION:          'metricsStartSession',
        constants.METHOD_SET_PROTOCOL_VERSION:   'metricsSetProtocolVersion',
        constants.METHOD_RECORD_DATA:            'metricsRecordData',
        constants.METHOD_OBSERVE_SESSION:        'metricsObserveSession',
        constants.METHOD_STOP_OBSERVING_SESSION: 'metricsStopObservingSession',
        constants.METHOD_RETRIEVE_SESSION:       'metricsRetrieveSession',
        constants.METHOD_DELETE_SESSION:         'metricsDeleteSession',
        constants.METHOD_LIST_SESSIONS:          'metricsListSessions',
    }


    def __init__(self, factory):
        protocol.MetricsProtocolBase.__init__(self)
        self.factory = factory
        self.session = None
        self.protocolVersion = 0


    def connectionLost(self, reason):
        log.msg("Client disconnected: %s" % reason)
        if self.session:
            self.session.close()
            self.session = None


    def metricsSetAuthorizationCredentials(self, correlationID, payload):
        pass


    def metricsStartSession(self, correlationID, payload):
        if not self.session:
            (sessionID,) = framing.decodePayload(payload)
            log.msg("Starting session %s" % sessionID)
            self.session = self.factory.sessionStore.createNewSession(sessionID)
            return (True, constants.RESULT_NONE, '')
        else:
            return (False, constants.ERROR_ALREADY_IN_SESSION, '')


    def metricsSetProtocolVersion(self, correlationID, payload):
        (self.protocolVersion,) = framing.decodePayload(payload)
        return (True, constants.RESULT_NONE, '')


    def metricsRecordData(self, correlationID, buffer):
        self.session.recordData(buffer)
        # No result so that we don't have a lot of useless traffic. Errors in
        # recording data need to be dealt with in some other manner (like
        # raising an error, which will send something back down to the client).
        return None


    def metricsObserveSession(self, correlationID, payload):
        pass


    def metricsStopObservingSession(self, correlationID, payload):
        pass


    def metricsRetrieveSession(self, correlationID, payload):
        pass


    def metricsDeleteSession(self, correlationID, payload):
        pass


    def metricsListSessions(self, correlationID, payload):
        (flags,) = framing.decodePayload(payload)
        sessions = self.factory.sessionStore.getSessions(**flags)
        log.msg("Sending sessions... %s" % sessions)
        self._sendSuccess(correlationID, constants.RESULT_DATA_FOLLOWS, '')
        self.sendCommand(correlationID, constants.METHOD_RECEIVE_SESSION_LIST_COUNT, framing.encodePayload(len(sessions)))
        for session in sessions:
            self.sendCommand(correlationID, constants.METHOD_RECEIVE_SESSION_LIST_ITEM, framing.encodePayload(session.sessionID))
        return None


