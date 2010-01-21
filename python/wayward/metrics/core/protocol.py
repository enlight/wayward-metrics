# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

from twisted.protocols import basic
from twisted.python import log
from wayward.metrics.core import constants, framing

class MetricsProtocolBase(basic.Int32StringReceiver):
    def __init__(self):
        pass


    def sendCommand(self, messageType, correlationID, payload):
        self.sendString(framing.encode(messageType, correlationID, payload))


    def stringReceived(self, msg):
        try:
            (messageType, correlationID, payload) = framing.decode(msg)
            methodName = self.dispatchTable.get(messageType)
            # log.msg("Executing %s:%s(%s)" % (messageType, methodName, payload))
            if methodName:
                f = getattr(self, methodName)
                result = f(correlationID, payload)
                if result:
                    success, resultCode, message = result
                    if success:
                        self._sendSuccess(correlationID, resultCode, message)
                    else:
                        self._sendFailure(correlationID, resultCode, message)
            else:
                self._sendFailure(correlationID, constants.ERROR_UNKNOWN_METHOD, methodName)
        except:
            log.err()
            self._sendFailure(correlationID, constants.ERROR_UNKNOWN, '')


    def metricsReceiveResult(self, correlationID, payload):
        log.msg("Result: %s" % repr(payload))


    def _sendSuccess(self, correlationID, resultCode, message):
        self.sendCommand(constants.METHOD_RECEIVE_RESULT, correlationID,
                         framing.encodePayload(True, resultCode, message))


    def _sendFailure(self, correlationID, resultCode, message):
        self.sendCommand(constants.METHOD_RECEIVE_RESULT, correlationID,
                         framing.encodePayload(False, resultCode, message))


