#import <CoreMIDI/CoreMIDI.h>
#include <iterator>
#include <list>
#include <queue>
#include <string>

#pragma mark Private classes

namespace
{
    // MIDI message structure.
    struct Message
    {
        Byte status;
        Byte data[2];
        
        Message(Byte status, Byte data1, Byte data2)
        :   status(status)
        {
            data[0] = data1;
            data[1] = data2;
        }
        
        operator UInt32() const
        {
            return static_cast<UInt32>(status) |
                (static_cast<UInt32>(data[0]) << 8) |
                (static_cast<UInt32>(data[1]) << 16);
        }
    };
    
    // MIDI endpoint class.
    class Endpoint
    {
    public:
        Endpoint()
        {
        }
        
        Endpoint(SInt32 id, const std::string name)
        :   id(id), name(name)
        {
        }
        
        bool operator == (SInt32 id) const
        {
            return this->id == id;
        }
        
        SInt32 id;
        std::string name;
        std::queue<Message> messageQueue;
    };
    
    // Endpoint list.
    std::list<Endpoint> endpoints;
}

#pragma mark Core MIDI callback function

namespace
{
    extern "C" void MyReadProc(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon)
    {
        Endpoint& endpoint = *reinterpret_cast<Endpoint*>(srcConnRefCon);
        // Queue the all incoming packets.
        for (int i = 0; i < pktlist->numPackets; i++) {
            const MIDIPacket *pkt = &pktlist->packet[i];
            endpoint.messageQueue.push(Message(pkt->data[0], pkt->data[1], pkt->data[2]));
        }
    }
}

#pragma mark Exposed functions

// Initializes the plug-in.
extern "C" void UnityMIDIReceiver_Initialize()
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    
    MIDIClientRef client;
    MIDIPortRef inputPort;
    
    // Create a MIDI client.
    MIDIClientCreate(CFSTR("MIDI Tester Client"), nil, nil, &client);
    
    // Create a MIDI port which covers all MIDI sources.
    ItemCount sourceCount = MIDIGetNumberOfSources();
    MIDIInputPortCreate(client, CFSTR("MIDI Tester Input Port"), MyReadProc, nil, &inputPort);
    
    // Enumerate the all MIDI sources.
    for (int i = 0; i < sourceCount; i++) {
        MIDIEndpointRef source = MIDIGetSource(i);
        
        // Retrieve the unique ID of this MIDI source.
        SInt32 uniqueID;
        MIDIObjectGetIntegerProperty(source, kMIDIPropertyUniqueID, &uniqueID);

        // Retrieve the redable name of this MIDI source.
        char modelName[256];
        CFStringRef modelStringRef;
        MIDIObjectGetStringProperty(source, kMIDIPropertyModel, &modelStringRef);
        CFStringGetCString(modelStringRef, modelName, sizeof(modelName), kCFStringEncodingUTF8);

        // Add this MIDI source to the endpoint list.
        endpoints.push_back(Endpoint(uniqueID, modelName));
        
        // Connect the MIDI source to the input port.
        MIDIPortConnectSource(inputPort, source, &endpoints.back());
    }
}

// Counts the number of endpoints.
extern "C" int UnityMIDIReceiver_CountEndpoints()
{
    return endpoints.size();
}

// Get the unique ID of an endpoint, with an index on the endpoint list.
extern "C" unsigned int UnityMIDIReceiver_GetEndpointIDAtIndex(int index)
{
    auto it = endpoints.begin();
    std::advance(it, index);
    return it->id;
}

// Get the name of an endpoint.
extern "C" const char* UnityMIDIReceiver_GetEndpointName(unsigned int id)
{
    auto it = std::find(endpoints.begin(), endpoints.end(), static_cast<SInt32>(id));
    return (it != endpoints.end()) ? it->name.c_str() : nullptr;
}

// Retrieve and erase an MIDI message data from an endpoint message queue.
extern "C" unsigned int UnityMIDIReceiver_DequeueMessageDataOnEndpoint(unsigned int id)
{
    auto it = std::find(endpoints.begin(), endpoints.end(), static_cast<SInt32>(id));
    if (it == endpoints.end()) return 0;
    if (it->messageQueue.empty()) return 0;

    Message m = it->messageQueue.back();
    it->messageQueue.pop();
    
    return static_cast<UInt32>(m);
}
