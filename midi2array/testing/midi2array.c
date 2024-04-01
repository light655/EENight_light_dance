#include "stdio.h"

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

int main(int argc, char** argv)
{
    FILE *input, *out_h, *out_py;			// pointer to files
    // char filename[20] = "WakaWakaEE_6.mid";
    // char filename[20] = "test.mid";
    char filename[20] = "test1.mid";
    unsigned char buffer[BUFFER_SIZE] = {0}; // buffer for input data
    int fc = 0, i;                           // file byte counter, looping index
    int t = 0;                               // absolute time in millisecond
    int n_track;                             // number of tracks in the midi file
    int tick_per_beat;                       // number of ticks in a beat
    int us_per_beat;                         // microsecond per beat in the midi file
    int delta_tick;                          // the time difference after the last note event in tick
    float us_per_tick;                       // microsecond of a tick
    int change_tempo = 0;                    // flag to indicate tempo just changed

    int a = 0, b = 0, c = 0, d = 0;   // indices for arrays for each light
    int A_t[ARRAY_SIZE] = {0}; // array of absolute time for each light
    int B_t[ARRAY_SIZE] = {0};
    int C_t[ARRAY_SIZE] = {0};
    int D_t[ARRAY_SIZE] = {0};
    char A_s[ARRAY_SIZE] = {0}; // array of state of each light
    char B_s[ARRAY_SIZE] = {0};
    char C_s[ARRAY_SIZE] = {0};
    char D_s[ARRAY_SIZE] = {0};

    // filename[11] = argv[1][0];
    input = fopen(filename, "r"); // open midi file

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
    n_track = buffer[BUFFER_SIZE - 4]; // extract bytes for n_track
    n_track <<= 8;
    n_track += buffer[BUFFER_SIZE - 3];
    tick_per_beat = buffer[BUFFER_SIZE - 2]; // extract bytes for tick_per_beat
    tick_per_beat <<= 8;
    tick_per_beat += buffer[BUFFER_SIZE - 1];
    printf("There are %d track(s). Every beat is %d ticks.\n", n_track, tick_per_beat);

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

    // find end of first note (A4 for 4 beats) to skip bytes I cannot decode
    while (litncmp(&buffer[BUFFER_SIZE - 3], "\x90\x45\x00", 3) &&
        litncmp(&buffer[BUFFER_SIZE - 3], "\x95\x45\x00", 3))
    {
        readNext(buffer, input);
        printf("%d\n", fc++);
    }
    printf("Found end of first note (A4 for 4 beats) to skip bytes I cannot decode at %d.\n", fc);

    us_per_tick = (float)us_per_beat / (float)tick_per_beat;
    do
    { // loop until end of file
        if(!readNext(buffer, input)) break;
        fc++;

        delta_tick = 0;
        // the byte is a part of delta time and is not the last byte
        while (buffer[BUFFER_SIZE - 1] & 0x80)
        {                                                  // the most significant bit is 1
            delta_tick += buffer[BUFFER_SIZE - 1] & ~0x80; // clear first bit, store into delta_tick
            delta_tick <<= 7;                              // shift left by 7 bits
            readNext(buffer, input);                       // read next byte
            fc++;
        }
        delta_tick += buffer[BUFFER_SIZE - 1];
        printf("Delta tick: %d\n", delta_tick);
        t += delta_tick * us_per_tick;

        if (change_tempo)
        { // read extra 0x90
            readNext(buffer, input);
            fc++;
            change_tempo = 0; // clear flag
        }
        readNext(buffer, input); // read note
        fc++;
        readNext(buffer, input); // read strength
        fc++;

        printf("%u\n", buffer[BUFFER_SIZE - 2]);
        printf("%d, %d\n", fc, t);
        switch (buffer[BUFFER_SIZE - 2])
        {
        case 0xff:
            if (buffer[BUFFER_SIZE - 1] == 0x51)
            {
                change_tempo = 1; // set flag
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
        case 0xb0:      // ignore 3 bytes
            printf("0xB0\n");
            readNext(buffer, input);	
            change_tempo = 1;
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
    } while (litncmp(&buffer[BUFFER_SIZE - 2], "\xff\x2f", 2));
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
