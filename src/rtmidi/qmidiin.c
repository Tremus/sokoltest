// qmidiin.c
 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "rtmidi_c.h"

#ifdef _WIN32
void Sleep(unsigned long ms);
#define SLEEP(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP(ms) usleep(ms * 1000)
#endif
 
bool done;
static void finish(int ignore){ done = true; }

int main()
{
    unsigned char messages[1024];
    size_t nBytes;
    struct RtMidiWrapper *midiin;
    float stamp;
    int i;
    if ((midiin = rtmidi_in_create_default())) {
        unsigned int nPorts;
        int portNameLen; 
        char portName[128];

        // Check available ports.
        nPorts = rtmidi_get_port_count(midiin);
        if ( nPorts == 0 ) {
            printf("No ports available!\n");
            goto cleanup;
        }
        rtmidi_get_port_name(midiin, 0, portName, &portNameLen);
        rtmidi_open_port(midiin, 0, "RtMidi");

         // Don't ignore sysex, timing, or active sensing messages.
        rtmidi_in_ignore_types(midiin, false, false, false);
        // Install an interrupt handler function.
        (void) signal(SIGINT, finish);

        // Periodically check input queue.
        printf("Reading MIDI from port %s quit with Ctrl-C.\n", portName);
        while (midiin->ok) {
            stamp = (float)rtmidi_in_get_message(midiin, messages, &nBytes);

            for ( i=0; i<nBytes; i++ )
                printf("Byte %d = %d, ", i, (int)messages[i]);
            if ( nBytes > 0 )
                printf("stamp = %.2f\n", stamp);

            SLEEP(10);
        }
        rtmidi_close_port(midiin);

    cleanup:
        rtmidi_in_free(midiin);
    }
 
  return 0;
}