#import <Foundation/Foundation.h>
#import <CoreMIDI/CoreMIDI.h>

static MIDIClientRef client;
static MIDIPortRef inputPort;
static NSMutableString *logText;

static void MyReadProc(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon)
{
    // Process the all incoming packets.
    for (int i = 0; i < pktlist->numPackets; i++) {
        const MIDIPacket *pkt = &pktlist->packet[i];
        for (int offs = 0; offs < pkt->length; offs++) {
            [logText appendFormat:@" %02X", pkt->data[offs]];
        }
        [logText appendString:@"\n"];
    }
}

char *UnityMIDIReceiver_GetLogText()
{
    const char *text = logText.UTF8String;
    char *copiedText = static_cast<char*>(malloc(strlen(text)));
    return strcpy(copiedText, text);
}

void UnityMIDIReceiver_Initialize()
{
    logText = [NSMutableString stringWithCapacity:64 * 1024];
    
    // Create a MIDI client.
    MIDIClientCreate((CFStringRef)@"MIDI Tester Client", nil, nil, &client);
    
    // Create a MIDI port which covers all MIDI sources.
    ItemCount sourceCount = MIDIGetNumberOfSources();
    MIDIInputPortCreate(client, (CFStringRef)@"MIDI Tester Input Port", MyReadProc, nil, &inputPort);
    
    // Enumerate the all MIDI sources.
    for (int i = 0; i < sourceCount; i++) {
        MIDIEndpointRef source = MIDIGetSource(i);
        
        // Retrieve the name of the MIDI source.
        CFStringRef modelStringRef;
        CFStringRef nameStringRef;
        MIDIObjectGetStringProperty(source, kMIDIPropertyModel, &modelStringRef);
        MIDIObjectGetStringProperty(source, kMIDIPropertyName, &nameStringRef);
        NSString *name = [NSString stringWithFormat:@"%@ (%@)", nameStringRef, modelStringRef];
        
        [logText appendFormat:@"A MIDI source was found: %@\n", name];
        
        // Connect the MIDI source to the input port.
        MIDIPortConnectSource(inputPort, source, nil);
    }
}
