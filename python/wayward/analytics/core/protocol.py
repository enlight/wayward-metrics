# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

from twisted.protocols import basic
from twisted.python import log
from wayward.analytics.core import constants, framing

class AnalyticsProtocolBase(basic.Int32StringReceiver):
    def __init__(self):
        pass


    def sendCommand(self, messageType, payload):
        self.sendString(framing.encode(messageType, payload))


    def stringReceived(self, msg):
        try:
            (messageType, payload) = framing.decode(msg)
            methodName = self.dispatchTable.get(messageType)
            # log.msg("Executing %s:%s(%s)" % (messageType, methodName, payload))
            if methodName:
                f = getattr(self, methodName)
                result = f(payload)
                if result:
                    success, resultCode, message = result
                    if success:
                        self._sendSuccess(resultCode, message)
                    else:
                        boundError = True
                        if messageType == constants.METHOD_RECORD_DATA:
                            boundError = False
                        self._sendFailure(resultCode, message, boundError)
            else:
                self._sendFailure(constants.ERROR_UNKNOWN_METHOD, methodName, True)
        except:
            log.err()
            self._sendFailure(constants.ERROR_UNKNOWN, '', False)


    def analyticsReceiveResult(self, payload):
        log.msg("Result: %s" % repr(payload))


    def _sendSuccess(self, resultCode, message):
        self.sendCommand(constants.METHOD_RECEIVE_RESULT, framing.encodePayload(True, resultCode, message))


    def _sendFailure(self, resultCode, message, failureIsBoundToCommand):
        self.sendCommand(constants.METHOD_RECEIVE_RESULT,
                         framing.encodePayload(False, resultCode, message, failureIsBoundToCommand))


