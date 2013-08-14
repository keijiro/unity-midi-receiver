unity-midi-receiver
===================

This is a minimal implementation of MIDI receiver plug-in for Unity.
It can only handle incoming messages from MIDI devices
which is connected to the host computer.

Supported Platform
------------------
- Mac OS X 10.6 or later
- Unity 4 Pro
- Universal binary (runs on both x86 and x64)

The Windows version is [here]
(https://github.com/keijiro/unity-midi-receiver-windows).

Basic Usage
-----------

1. In Update function, call **UnityMIDIReceiver_DequeueIncomingData**
   until it returns zero.
1. Extract MIDI messages from incoming data,
   and use it for something interesting.

For the detailed usage, see [a sample program]
(https://github.com/keijiro/unity-midi-receiver-test).

Function Reference
------------------

#### UnityMIDIReceiver_CountEndpoints

Counts the number of endpoints (MIDI sources).

#### UnityMIDIReceiver_GetEndpointIDAtIndex

Returns the ID of the endpoint at a given index.

#### UnityMIDIReceiver_GetEndpointName

Retrieves the display name of the endpoint. It returns a reference pointer
to the name as a IntPtr value, and you can use Marshal.PtrToStringAnsi to
convert it into C# string.

#### UnityMIDIReceiver_DequeueIncomingData

Removes and return the oldest incoming message on the message queue.
Message data is encoded in 64-bit integer value.

Incoming Data Structure
-----------------------

Incoming data is encoded in 64-bit integer value.
You can extract the information about the source endpoint
and a MIDI message.

| 63 - 56 |    55 - 48    |    47 - 40    |   39 - 32   |   31 - 0    |
| ------- | ------------- | ------------- | ----------- | ----------- |
| (zero)  | MIDI data (2) | MIDI data (1) | MIDI status | Endpoint ID |

License
-------

Copyright (C) 2013 Keijiro Takahashi

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
