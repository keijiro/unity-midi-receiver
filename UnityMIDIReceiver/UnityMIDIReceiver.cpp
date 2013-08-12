#import <CoreMIDI/CoreMIDI.h>

static MIDIClientRef client;
static MIDIPortRef inputPort;
static char logText[256 * 1024];

static void MyReadProc(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon)
{
    // Process the all incoming packets.
    for (int i = 0; i < pktlist->numPackets; i++) {
        const MIDIPacket *pkt = &pktlist->packet[i];
        for (int offs = 0; offs < pkt->length; offs++) {
            char temp[32];
            sprintf(temp, " %02X", pkt->data[offs]);
            strcat(logText, temp);
        }
        strcat(logText, "\n");
    }
}

extern "C" const char *UnityMIDIReceiver_GetLogText()
{
    return logText;
}

extern "C" void UnityMIDIReceiver_Initialize()
{
    // Create a MIDI client.
    MIDIClientCreate(CFSTR("MIDI Tester Client"), nil, nil, &client);
    
    // Create a MIDI port which covers all MIDI sources.
    ItemCount sourceCount = MIDIGetNumberOfSources();
    MIDIInputPortCreate(client, CFSTR("MIDI Tester Input Port"), MyReadProc, nil, &inputPort);
    
    // Enumerate the all MIDI sources.
    for (int i = 0; i < sourceCount; i++) {
        MIDIEndpointRef source = MIDIGetSource(i);
        
        // Retrieve the name of the MIDI source.
        CFStringRef modelStringRef;
        CFStringRef nameStringRef;
        MIDIObjectGetStringProperty(source, kMIDIPropertyModel, &modelStringRef);
        MIDIObjectGetStringProperty(source, kMIDIPropertyName, &nameStringRef);
        
        strcat(logText, "A MIDI source was found: ");
        CFStringGetCString(modelStringRef, logText + strlen(logText), sizeof(logText) - strlen(logText), kCFStringEncodingUTF8);
        strcat(logText, " - ");
        CFStringGetCString(nameStringRef, logText + strlen(logText), sizeof(logText) - strlen(logText), kCFStringEncodingUTF8);
        strcat(logText, "\n");
        
        // Connect the MIDI source to the input port.
        MIDIPortConnectSource(inputPort, source, nil);
    }
}
