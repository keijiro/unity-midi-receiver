unity-midi-receiver
===================

This is a minimal implementation of MIDI receiver plug-in for Unity.
It can only handle incoming messages from MIDI devices
which is connected to the host computer.

Basic Usage
-----------

1. Call **UnityMIDIReceiver_Initialize** in a Start function.
1. In an Update function, call **UnityMIDIReceiver_DequeueIncomingData**
   until it returns zero.
1. Extract MIDI messages from incoming data,
   and use it for something interesting.

For the detailed usage, see [a sample program]
(https://github.com/keijiro/unity-midi-receiver-test).

Function Reference
------------------

#### UnityMIDIReceiver_Initialize

Initializes/resets the plug-in. You can call this function as many you need.

#### UnityMIDIReceiver_CountEndpoints

Counts the number of endpoints (MIDI sources).

#### UnityMIDIReceiver_GetEndpointIDAtIndex

Returns the ID of the endpoint at a given index.

#### UnityMIDIReceiver_GetEndpointName

Retrieves the display name of the endpoint.

#### UnityMIDIReceiver_DequeueIncomingData

Removes and return the oldest incoming message on the message queue.
Message data is encoded in 64-bit integer value.

Incoming Data Structure
-----------------------

Incoming data is encoded in 64-bit integer value.
You can extract the information about the source endpoint
and a MIDI message.

|   63 - 32   | 31 - 24 |    23 - 16    |    15 - 8     |    7 - 0    |
| ----------- | ------- | ------------- | ------------- | ----------- |
| Endpoint ID | (zero)  | MIDI data (2) | MIDI data (1) | MIDI status |

Development status
------------------

- It only supports Mac OS X at the moment.
  Windows support is not ready yet.
