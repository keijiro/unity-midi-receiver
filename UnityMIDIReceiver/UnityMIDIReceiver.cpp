#import <CoreMIDI/CoreMIDI.h>
#include <queue>
#include <mutex>

#pragma mark Private classes

namespace
{
    // MIDI message structure.
    union Message {
        uint64_t uint64Value;
        
        struct
        {
            MIDIUniqueID source;
            Byte status;
            Byte data1;
            Byte data2;
        };
        
        Message(MIDIUniqueID aSource, Byte aStatus, Byte aData1, Byte aData2)
        :   source(aSource), status(aStatus), data1(aData1), data2(aData2)
        {
        }
    };
    
    static_assert(sizeof(Message) == sizeof(uint64_t), "Wrong data size.");

    // MIDI source ID array.
    MIDIUniqueID sourceIDs[256];

    // Incoming MIDI message queue.
    std::queue<Message> messageQueue;
    std::mutex messageQueueLock;

    // Core MIDI objects.
    MIDIClientRef client;
    MIDIPortRef inputPort;
    
    // Reset-is-required flag.
    bool resetIsRequired = true;
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
        
        // Order to reset the plug-in.
        resetIsRequired = true;
    }
    
    extern "C" void MyMIDIReadProc(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon)
    {
        auto sourceID = *reinterpret_cast<MIDIUniqueID*>(srcConnRefCon);
        
        messageQueueLock.lock();
        
        // Transform the packets into MIDI messages and push it to the message queue.
        for (int i = 0; i < pktlist->numPackets; i++) {
            const MIDIPacket& packet = pktlist->packet[i];
            messageQueue.push(Message(sourceID, packet.data[0], packet.data[1], packet.data[2]));
        }
        
        messageQueueLock.unlock();
    }
}

#pragma mark Private functions
    
namespace
{
    void ResetPluginIfRequired()
    {
        if (!resetIsRequired) return;
        
        // Dispose the old MIDI client if exists.
        if (client != 0) MIDIClientDispose(client);
        
        // Clear the message queue.
        std::queue<Message> emptyQueue;
        std::swap(messageQueue, emptyQueue);
        
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
        
        resetIsRequired = false;
    }
}

#pragma mark Exposed functions

// Counts the number of endpoints.
extern "C" int UnityMIDIReceiver_CountEndpoints()
{
    ResetPluginIfRequired();
    return static_cast<int>(MIDIGetNumberOfSources());
}

// Get the unique ID of an endpoint.
extern "C" uint32_t UnityMIDIReceiver_GetEndpointIDAtIndex(int index)
{
    return sourceIDs[index];
}

// Get the name of an endpoint.
extern "C" const char* UnityMIDIReceiver_GetEndpointName(uint32_t id)
{
    ResetPluginIfRequired();

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
    ResetPluginIfRequired();

    if (messageQueue.empty()) return 0;
    
    messageQueueLock.lock();
    Message m = messageQueue.back();
    messageQueue.pop();
    messageQueueLock.unlock();
    
    return m.uint64Value;
}
