// qmidiin.c
 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "rtmidi_c.h"

#ifdef _WIN32
extern void Sleep(unsigned long ms);
#define SLEEP(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP(ms) usleep(ms * 1000)
#endif

enum MidiEventType {
    MIDI_NOTE_OFF         = 0x80,
    MIDI_NOTE_ON          = 0x90,
};

bool done = false;
static void finish(int ignore){ done = true; }

int main() {
    unsigned char messages[1024];
    size_t nBytes;
    struct RtMidiWrapper *midiin;
    float stamp;
    int i;
    unsigned int nPorts;
    int portNameLen; 
    char portName[128];

    midiin = rtmidi_in_create_default();
    if (midiin == NULL) {
        printf("Failed initialising RtMidi!\n");
        return 1;
    }

    // Check available ports.
    nPorts = rtmidi_get_port_count(midiin);
    if (nPorts == 0) {
        printf("No ports available!\n");
        return 1;
    }
    rtmidi_get_port_name(midiin, 0, portName, &portNameLen);
    rtmidi_open_port(midiin, 0, "RtMidi");

    // Don't ignore sysex, timing, or active sensing messages.
    rtmidi_in_ignore_types(midiin, false, false, false);
    // Install an interrupt handler function.
    (void) signal(SIGINT, finish);

    // Periodically check input queue.
    printf("Reading MIDI from port %s quit with Ctrl-C.\n", portName);
    while (midiin->ok && !done) {
        do {
            nBytes = sizeof(messages);
            stamp = (float)rtmidi_in_get_message(midiin, messages, &nBytes);

            if ( nBytes > 0 )
            {
                unsigned char* readbuf = messages;
                unsigned char status = *readbuf;
                printf("in bytes %u, stamp = %.2f\n", (unsigned)nBytes, stamp);

                if ((status & 0xf0) == MIDI_NOTE_ON) {
                    unsigned channel  = status & 0x0f;
                    unsigned note     = readbuf[1];
                    unsigned velocity = readbuf[2];
                    printf("note on! channel: %u, note: %u, velocity: %u\n", channel, note, velocity);
                } else if ((status & 0xf0) == MIDI_NOTE_OFF) {
                    unsigned channel  = status & 0x0f;
                    unsigned note     = readbuf[1];
                    unsigned velocity = readbuf[2];
                    printf("note off... channel: %u, note: %u, velocity: %u\n", channel, note, velocity);
                }
            }
        } while (nBytes != 0 && !done);

        SLEEP(10);
    }
    rtmidi_close_port(midiin);

    // OS cleans up automatically when process exits...
    // rtmidi_in_free(midiin); 
    return 0;
}