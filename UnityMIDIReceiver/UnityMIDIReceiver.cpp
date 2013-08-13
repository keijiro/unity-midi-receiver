#import <CoreMIDI/CoreMIDI.h>
#include <queue>

#pragma mark Private classes

namespace
{
    // MIDI message structure.
    // TODO: Use union.
    struct Message
    {
        MIDIUniqueID source;
        Byte status;
        Byte data1;
        Byte data2;
        
        Message(MIDIUniqueID source, Byte status, Byte data1, Byte data2)
        :   source(source), status(status), data1(data1), data2(data2)
        {
        }
        
        operator uint64_t() const
        {
            return
                (static_cast<uint64_t>(source) & 0xffffffffULL) |
                (static_cast<uint64_t>(status) << 32) |
                (static_cast<uint64_t>(data1) << 40) |
                (static_cast<uint64_t>(data2) << 48);
        }
    };

    // MIDI source ID array.
    MIDIUniqueID sourceIDs[256];

    // Incoming MIDI message queue.
    std::queue<Message> messageQueue;

    // Core MIDI objects.
    MIDIClientRef client;
    MIDIPortRef inputPort;
    
    // Local functions.
    void ReconnectAllSources();
}

#pragma mark Core MIDI callback function

namespace
{
    extern "C" void MyMIDIStateChangedHander(const MIDINotification* message, void* refCon)
    {
        // Only process additions and removals.
        if (message->messageID != kMIDIMsgObjectAdded && message->messageID != kMIDIMsgObjectRemoved) return;
        
        // Only process source operations.
        auto addRemoveDetail = reinterpret_cast<const MIDIObjectAddRemoveNotification*>(message);
        if (addRemoveDetail->childType != kMIDIObjectType_Source) return;
        
        // Update the MIDI client state.
        ReconnectAllSources();
    }
    
    extern "C" void MyMIDIReadProc(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon)
    {
        auto sourceID = *reinterpret_cast<MIDIUniqueID*>(srcConnRefCon);
        
        // Transform the packets into MIDI messages and push it to the message queue.
        for (int i = 0; i < pktlist->numPackets; i++) {
            const MIDIPacket& packet = pktlist->packet[i];
            messageQueue.push(Message(sourceID, packet.data[0], packet.data[1], packet.data[2]));
        }
    }
}

#pragma mark Private functions
    
namespace
{
    void ReconnectAllSources()
    {
        // Dispose the old MIDI client if exists.
        if (client != nullptr) MIDIClientDispose(client);
        
        // Create a MIDI client.
        MIDIClientCreate(CFSTR("UnityMIDIReceiver Client"), MyMIDIStateChangedHander, nullptr, &client);
        
        // Create a MIDI port which covers all MIDI sources.
        MIDIInputPortCreate(client, CFSTR("UnityMIDIReceiver Input Port"), MyMIDIReadProc, nullptr, &inputPort);
        
        // Enumerate the all MIDI sources.
        ItemCount sourceCount = MIDIGetNumberOfSources();
        assert(sourceCount < sizeof(sourceIDs));
        
        for (int i = 0; i < sourceCount; i++) {
            // Connect the MIDI source to the input port.
            MIDIEndpointRef source = MIDIGetSource(i);
            MIDIObjectGetIntegerProperty(source, kMIDIPropertyUniqueID, &sourceIDs[i]);
            MIDIPortConnectSource(inputPort, source, &sourceIDs[i]);
        }
    }
}

#pragma mark Exposed functions

// Initializes the plug-in.
extern "C" void UnityMIDIReceiver_Initialize()
{
    ReconnectAllSources();
}

// Counts the number of endpoints.
extern "C" int UnityMIDIReceiver_CountEndpoints()
{
    return MIDIGetNumberOfSources();
}

// Get the unique ID of an endpoint.
extern "C" uint32_t UnityMIDIReceiver_GetEndpointIDAtIndex(int index)
{
    return sourceIDs[index];
}

// Get the name of an endpoint.
extern "C" const char* UnityMIDIReceiver_GetEndpointName(uint32_t id)
{
    MIDIObjectRef object;
    MIDIObjectType type;
    MIDIObjectFindByUniqueID(id, &object, &type);
    assert(type == kMIDIObjectType_Source);
    
    CFStringRef name;
    MIDIObjectGetStringProperty(object, kMIDIPropertyDisplayName, &name);
    
    static char buffer[256];
    CFStringGetCString(name, buffer, sizeof(buffer), kCFStringEncodingUTF8);
    
    return buffer;
}

// Retrieve and erase an MIDI message data from the message queue.
extern "C" uint64_t UnityMIDIReceiver_DequeueIncomingData()
{
    if (messageQueue.empty()) return 0;

    Message m = messageQueue.back();
    messageQueue.pop();
    
    return static_cast<uint64_t>(m);
}
