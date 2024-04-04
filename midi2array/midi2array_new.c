#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_SIZE 50
#define BUFFER_SIZE 8
#define ARRAY_SIZE 1000
#define LINE_SIZE 10
#define NOTE_A 69u
#define NOTE_B 71u
#define NOTE_C 72u
#define NOTE_D 74u

int litncmp(unsigned char *array, char *lit, size_t n);
// compare char array with string literal (excluding termination null character)
int readNext(unsigned char *buffer, FILE *input);           // read next byte
void outputT(int *array, char *name, FILE *out_h, FILE *out_py);			// output the t array to .h file
void outputStrength(char *array, char *name, FILE *out_h, FILE *out_py);	// output the s array to .h file

// command line argument:
//      int track number
int main(int argc, char **argv) {
    FILE *input, *out_h, *out_py;			    // pointer to files
    char filename[STRING_SIZE] = "EENLD_X.mid"; // input filename
    unsigned char buffer[BUFFER_SIZE] = {0};    // buffer for input data
    int fc = 0, i;                              // file byte counter, looping index
    int t = 0;                                  // absolute time in millisecond
    unsigned char track_no;                     // track number
    int tick_per_beat;                          // number of ticks in a beat
    int us_per_beat;                            // microsecond per beat in the midi file
    int delta_tick;                             // the time difference after the last note event in tick
    float us_per_tick;                          // number of microsecond in a tick
    int note_flag = 0;                          // flag to indicate note on/off byte should not be ignored

    int a = 0, b = 0, c = 0, d = 0;   // indices for arrays for each light
    int A_t[ARRAY_SIZE] = {0}; // array of absolute time for each light
    int B_t[ARRAY_SIZE] = {0};
    int C_t[ARRAY_SIZE] = {0};
    int D_t[ARRAY_SIZE] = {0};
    char A_s[ARRAY_SIZE] = {0}; // array of state of each light
    char B_s[ARRAY_SIZE] = {0};
    char C_s[ARRAY_SIZE] = {0};
    char D_s[ARRAY_SIZE] = {0};

    // deal with opening file ----------------------------------------------------------------------
    if(argc == 1) {
        strncpy(filename, "test1.mid", STRING_SIZE - 1);  // use test input file if no command line argument provided
        track_no = 0;                       // default track number is 0
    } else {
        char input_filename[20];
        strncpy(input_filename, argv[1], STRING_SIZE - 1);
        
        if(strlen(input_filename) == 1) {   // set track number if command line argument is 1 character long
            filename[6] = argv[1][0];
            track_no = argv[1][0] - '0';
            printf("Track No.%d\n", track_no);
        } else {                            // use the filename in the command line argument if it is longer than 1 character
            strncpy(filename, input_filename, STRING_SIZE - 1); 
        }
    }
    printf("Using input file: %s\n", filename);
    input = fopen(filename, "r");   // open midi file

    // deal with decoding header chunk -------------------------------------------------------------
    while (litncmp(&buffer[BUFFER_SIZE - 4], "MThd", 4))
    { // find header chunk
        readNext(buffer, input);
        fc++;
    }

    for (i = 0; i < 10; i++)
    { // read all 10 bytes of the header chunk
        readNext(buffer, input);
        fc++;
    }
    tick_per_beat = buffer[BUFFER_SIZE - 2]; // extract bytes for tick_per_beat
    tick_per_beat <<= 8;
    tick_per_beat += buffer[BUFFER_SIZE - 1];
    printf("Every beat is %d ticks.\n", tick_per_beat);

    // deal with the initial settings of the track chunk -------------------------------------------
    while (litncmp(&buffer[BUFFER_SIZE - 4], "MTrk", 4))
    { // find track chunk
        readNext(buffer, input);
        fc++;
    }
    printf("Found track chunk at %d.\n", fc);

    while (litncmp(&buffer[BUFFER_SIZE - 3], "\xff\x51\x03", 3))
    { // find set tempo command
        readNext(buffer, input);
        fc++;
    }
    printf("Found set tempo command at %d. \n", fc);
    // read next 3 bytes, convert bytes into int
    readNext(buffer, input);
    us_per_beat = buffer[BUFFER_SIZE - 1];
    us_per_beat <<= 8;
    readNext(buffer, input);
    us_per_beat += buffer[BUFFER_SIZE - 1];
    us_per_beat <<= 8;
    readNext(buffer, input);
    us_per_beat += buffer[BUFFER_SIZE - 1];
    printf("Every beat is %d microseconds.\n", us_per_beat);

    us_per_tick = (float)us_per_beat / (float)tick_per_beat;

    // find end of first note (A4 for 4 beats) to skip bytes I cannot decode -----------------------
    char *tmp = malloc(sizeof(char) * 3);
    tmp[0] = 0x90 | track_no;
    tmp[1] = 0x45;
    tmp[2] = 0x00;
    while (litncmp(&buffer[BUFFER_SIZE - 3], tmp, 3)) {
        readNext(buffer, input);
        printf("%d\n", fc++);
    }
    printf("Found end of first note (A4 for 4 beats) to skip bytes I cannot decode at %d.\n", fc);
    free(tmp);

    // loop through the file -----------------------------------------------------------------------
    do {    
        if(!readNext(buffer, input)) break;     
            // read a byte into the buffer, 
            // if EOF is read, end the loop
        fc++;

        // the loop should begin with reading the delta tick ---------------------------------------
        delta_tick = 0;
        // delta time bytes have a 1 at the most significant bit except for the least significant byte
        while (buffer[BUFFER_SIZE - 1] & 0x80)  // this loop deals with the first few bytes
        {                                       // whose most significant bit is 1
            delta_tick += buffer[BUFFER_SIZE - 1] & ~0x80; // clear first bit, store into delta_tick
            delta_tick <<= 7;                              // shift left by 7 bits
            readNext(buffer, input);                       // read next byte
            fc++;
        }
        delta_tick += buffer[BUFFER_SIZE - 1];      // last byte of delta tick
        printf("Delta tick: %d\n", delta_tick);
        t += delta_tick * us_per_tick;

        // deal with the note on/off byte ----------------------------------------------------------
        // the note on/off byte will be dropped if the previous message is also a note on/off
        if(note_flag) {     // note on/off byte present flag
            readNext(buffer, input);    // read note on/off byte, do nothing
            fc++;
            note_flag = 0;              // clear flag
        }

        // read two bytes and branch on them -------------------------------------------------------
        // in the case of a note, the 1st byte is the note, the 2nd byte is the strength
        // in the case of a command, if we see 0xff, its a tempo change
        // if we see 0xb0, its something we don't know
        readNext(buffer, input);
        fc++;
        readNext(buffer, input);
        fc++;
        printf("%u\n", buffer[BUFFER_SIZE - 2]);
        printf("%d, %d\n", fc, t);
        switch (buffer[BUFFER_SIZE - 2]) {
        case 0xff:      // 0xff, typically is the change tempo command
            if (buffer[BUFFER_SIZE - 1] == 0x51)    // check 0x51 to comfirm it is a change tempo command
            {
                note_flag = 1;              // set flag
				readNext(buffer, input);	// 0x03 discard

                printf("Found set tempo command at %d. ", fc);
                // read next 3 bytes, convert bytes into int
                readNext(buffer, input);
                us_per_beat = buffer[BUFFER_SIZE - 1];
                us_per_beat <<= 8;
                readNext(buffer, input);
                us_per_beat += buffer[BUFFER_SIZE - 1];
                us_per_beat <<= 8;
                readNext(buffer, input);
                us_per_beat += buffer[BUFFER_SIZE - 1];
                printf("Every beat is %d microseconds.\n", us_per_beat);

                us_per_tick = (float)us_per_beat / (float)tick_per_beat;
            }
            break;
        case 0xb0:      // 0xb0, unknown command
            printf("0xb0\n");
            readNext(buffer, input);	// discard the unknown byte after 0xb0
            note_flag = 1;              // set flag
            break;
        case NOTE_A:
            A_t[a] = t;
            A_s[a] = (char)buffer[BUFFER_SIZE - 1];
            a++;
            break;
        case NOTE_B:
            B_t[b] = t;
            B_s[b] = (char)buffer[BUFFER_SIZE - 1];
            b++;
            break;
        case NOTE_C:
            C_t[c] = t;
            C_s[c] = (char)buffer[BUFFER_SIZE - 1];
            c++;
            break;
        case NOTE_D:
            D_t[d] = t;
            D_s[d] = (char)buffer[BUFFER_SIZE - 1];
            d++;
            break;
        default:
            break;
        }
    } while(litncmp(&buffer[BUFFER_SIZE - 2], "\xff\x2f", 2));  // loop if we haven't see the end of track bytes
    A_t[a] = -1; // set -1 to indicate end
    A_s[a] = -1;
    B_t[b] = -1;
    B_s[b] = -1;
    C_t[c] = -1;
    C_s[c] = -1;
    D_t[d] = -1;
    D_s[d] = -1;
    printf("Complete reading MIDI file.\n");
    fclose(input); // close midi file

    out_h = fopen("midi_array.h", "w"); // open output .h file
    out_py = fopen("midi_array.py", "w"); // open output .py file

    outputT(A_t, "A_t", out_h, out_py); // output the arrays
    outputStrength(A_s, "A_s", out_h, out_py);
    outputT(B_t, "B_t", out_h, out_py);
    outputStrength(B_s, "B_s", out_h, out_py);
    outputT(C_t, "C_t", out_h, out_py);
    outputStrength(C_s, "C_s", out_h, out_py);
    outputT(D_t, "D_t", out_h, out_py);
    outputStrength(D_s, "D_s", out_h, out_py);

    fclose(out_h); // close output .h file
    fclose(out_py); // close output .py file

    return 0;
}


// compare char array with string literal (excluding termination null character)
// parameters:
//		unsigned char *array: array of char to compare
//		char *lit: string literal to compare
//		size_t n: length of literal to compare
// return value:
//		type int, 0 if same.
// warning:
//		Make sure the avaliable chars to compare is longer than n!
int litncmp(unsigned char *array, char *lit, size_t n)
{
    int result = 0;
    int i = 0;

    while (result == 0 && i < n)
    {
        result = (int)array[i] - (unsigned char)lit[i];
        i++;
    }

    return result;
}

// function to read next byte from midi file
// parameters:
//		unsigned char *buffer: pointer to byte buffer
//		FILE *input: pointer to input file
// return value:
//		type int, 0 if reached EOF, 1 otherwise
int readNext(unsigned char *buffer, FILE *input)
{
    int i;  // looping index
    int ch; // temporary variable

    for (i = 0; i < BUFFER_SIZE - 1; i++) // move bytes in buffer forward
        buffer[i] = buffer[i + 1];
    if ((ch = getc(input)) != EOF)
    {
        buffer[BUFFER_SIZE - 1] = (char)ch; // read next byte from midi file
        return 1;
    }
    else
        return 0;
}

// function to output the dt array to .h file
// parameters:
//      int *array: array of absolute time, -1 terminated
//      char *name: name of the array in the output .h file
//      FILE *out_h: pointer to output .h file
//      FILE *out_py: pointer to output .py file
void outputT(int *array, char *name, FILE *out_h, FILE *out_py)
{
    int i; // looping index

    fprintf(out_h, "const int32_t %s[%d] = {", name, ARRAY_SIZE);
    fprintf(out_py, "%s = [", name);
    for (i = 0; array[i] != -1; i++)
    {
        if (i % LINE_SIZE == 0)
        {
            fprintf(out_h, "\n\t");
            fprintf(out_py, "\n\t");
        }

        if (i == 0)
        {
            fprintf(out_h, "%d, ", array[0]);
            fprintf(out_py, "%d, ", array[0]);
        }
        else
        {
            // fprintf(out_h, "%d, ", array[i] - array[i - 1]);
            fprintf(out_h, "%d, ", array[i]);
            fprintf(out_py, "%d, ", array[i]);
        }
    }
    fprintf(out_h, "-1\n"
                    "};\n"
                    "\n");
    fprintf(out_py, "-1\n"
                    "]\n"
                    "\n");
    return;
}

// function to output the s array to .h file
// parameters:
//      int *array: array of note strength time, -1 terminated
//      char *name: name of the array in the output .h file
//      FILE *out_h: pointer to output .h file
//      FILE *out_py: pointer to output .py file
void outputStrength(char *array, char *name, FILE *out_h, FILE *out_py)
{
    int i; // looping index

    fprintf(out_h, "const int32_t %s[%d] = {", name, ARRAY_SIZE);
    fprintf(out_py, "%s = [", name);
    for (i = 0; array[i] != -1; i++)
    {
        if (i % LINE_SIZE == 0)
        {
            fprintf(out_h, "\n\t");
            fprintf(out_py, "\n\t");
        }

        fprintf(out_h, "%d, ", array[i]);
        fprintf(out_py, "%d, ", array[i]);
    }
    fprintf(out_h, "-1\n"
                    "};\n"
                    "\n");
    fprintf(out_py, "-1\n"
                    "]\n"
                    "\n");

    return;
}
