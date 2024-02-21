# Reliability

## Packet layer

RTT should be measured for packets (so that's one reason to store entries for sent packets, at least some in a ring buffer).
When receiving ACKs the ACKed packets need to be signalled as ACKed (unless it's already been ACKed).
The message layer could perhaps check this instead of a more complex solution with callbacks? Or perhaps callbacks?

## Message layer

Reliably-sent messages must be stored with all its data and references to all the packets they have been sent in
(to ACK the message when the any of the packets it was sent in has been ACKed).