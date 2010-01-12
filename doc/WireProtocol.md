Wire Protocol
-------------

Framing
-------

Messages are transmitted on the network prefixed by their length as a 4 byte
integer in network order.

Messages themselves consist of:

   * 4 byte integer in network order for the message type.
   * 4 byte integer in network order for a correlation ID to tie
     requests to their responses.
   * A payload, encoded as described in a subsequent section.

Payload Encoding
----------------

This is currently using [BERT](http://bert-rpc.org/). We may change in the
future to [BSON](http://www.mongodb.org/display/DOCS/BSON) or something
else entirely.

Our priorities here are:
   * High performance for encoding and decoding. We pass around a lot
     of information, so we need as little overhead per message as we can
     get.
   * Small size for the encoded contents. A compact encoding is critical
     to keeping our archived data at a manageable size.
   * Accessible from multiple languages like C/C++, Python, Erlang, etc.


