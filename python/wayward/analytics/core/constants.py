# Copyright (c) 2010 Wayward Monkeys.
# See LICENSE for details.

"""
@author: Bruce Mitchener <bruce.mitchener@waywardmonkeys.com>
"""

METHOD_UNKNOWN                      = 0
METHOD_RECEIVE_RESULT               = 1

# Server implemented methods
METHOD_START_SESSION                = 101
METHOD_SET_PROTOCOL_VERSION         = 102
METHOD_RECORD_DATA                  = 103
METHOD_OBSERVE_SESSION              = 104
METHOD_STOP_OBSERVING_SESSION       = 105
METHOD_RETRIEVE_SESSION             = 106
METHOD_DELETE_SESSION               = 107
METHOD_LIST_SESSIONS                = 108

# Client implemented methods
METHOD_RECEIVE_SESSION_LIST_COUNT   = 1001
METHOD_RECEIVE_SESSION_LIST_ITEM    = 1002


RESULT_NONE                     = 0
RESULT_DATA_FOLLOWS             = 1

ERROR_NONE                      = RESULT_NONE
ERROR_UNKNOWN                   = -1
ERROR_UNKNOWN_METHOD            = -2
ERROR_ALEADY_IN_SESSION         = -3

