#include "engine.h"
MIDI_CREATE_DEFAULT_INSTANCE();

// C Ionian     { C maj;  D min;  E min;  F maj;  G maj;  A min;  B dim  }
const chord ionian[7]     = {{0, maj},    {2, minor}, {4, minor}, {5, maj},   {7, maj},   {9, minor}, {11, dim}};

// C Dorian     { C min;  D min;  D♯ maj; F maj;  G min;  A dim;  A♯ maj  }
const chord dorian[7]     = {{0, minor},  {2, minor}, {3, maj},   {5, maj},   {7, minor}, {9, dim},   {10, maj}};

//C Phrygian    { C min;  C♯ maj; D♯ maj; F min;  G dim;  G♯ maj; A♯ min  }
const chord phrygian[7]   = {{0, minor},  {1, maj},   {3, maj},   {5, minor}, {7, dim},   {8, maj},   {10, minor}};

//C Lydian      { C maj;  D maj;  E min;  F♯ dim; G maj;  A min;  B min   }
const chord lydian[7]     = {{0, maj},    {2, maj},   {4, minor}, {6, dim},   {7, maj},   {9, minor}, {11, minor}};

//C Mixolydian  { C maj;  D min;  E dim;  F maj;  G min;  A min;  A♯ maj  }
const chord mixolydian[7] = {{0, maj},    {2, minor}, {4, dim},   {5, maj},   {7, minor}, {9, minor}, {10, maj}};

//C Aeolian     { C min;  D dim;  D♯ maj; F min;  G min;  G♯ maj; A♯ maj  }
const chord aeolian[7]    = {{0, minor},  {2, dim},   {3, maj},   {5, minor}, {7, minor}, {8, maj},   {10, maj}};

//C harmonic    { C min;  D dim;  D♯ aug; F min;  G maj;  G♯ maj; B dim   }
const chord harmonic[7]   = {{0, minor},  {2, dim},   {3, aug},   {5, minor}, {7, maj},   {8, maj},   {11, dim}}; 

//C Locrian     { C dim;  C♯ maj; D♯ min; F min;  F♯ maj; G♯ maj; A♯ min  }
const chord locrian[7]    = {{0, dim},    {1, maj},   {3, minor}, {5, minor}, {6, maj},   {8, maj},   {10, minor}};

const chord *all_chords[8] = {ionian, dorian, phrygian, lydian, mixolydian, aeolian, harmonic, locrian};

// Arrange the N elements of ARRAY in random order.  
void shuffle(int *array, size_t n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

void arp::setupArp(short bn, short bo, short os, unsigned short st, unsigned int d, int m, unsigned imode)
{
  baseNote = (notes) map(bn, 0, 800, 0, 11);
  baseOctave = (short) map(bo, 0, 800, 0, 7);
  octaveShift = (short) map(os, 0, 800, 0, 7);
  steps = (unsigned short) map(st, 0, 800, 1, 6);
  indelay = (unsigned int) map(d, 0, 800, 0, 500);
  mode = all_chords[(unsigned int) map(imode, 0, 800, 0, 8)];
  order = (unsigned int) map(m, 0, 800, 0, 4);
}

int arp::setProgression(unsigned int p)
{
    if (p<8)
    {
        progression = p;
        return 0;
    }
    else
        return -1;
}

int* createChord(notes root, chord_types i, int *notes_array, unsigned short *sh1, unsigned short *sh2)
{
    *sh1=0; *sh2=0;
    int s1, s2;
    switch (i) {
        case maj:   s1 = 4; s2 = 7; break;
        case minor: s1 = 3; s2 = 7; break;
        case dim:   s1 = 3; s2 = 6; break;
        case aug:   s1 = 4; s2 = 8; break;
    }
    notes_array[0] = root;
    notes_array[1] = (root+s1)%12;
    if (root+s1>11) *sh1=1; // octave shift
    notes_array[2] = (root+s2)%12;
    if (root+s1>11) *sh2=1; // octave shift
}

short midiByNote (notes note, short octave)
{
    if ((octave < -1)||(octave > 8))
        return -1;

    return (octave+1)*12 + (int)note;
}

void arp::play()
{
    int i;
    short n, shift, oct_shift, bn;
    unsigned short sh1, sh2, notes_added=0;
    notes curNote[3];
    short notestoplay[15] = {0,0,0,0,0,0,0,0,0,0};
    // Play base note
    if ((int)baseNote + (int)mode[progression].Shift > 11)
        baseOctave++;
    i = (baseNote + mode[progression].Shift)%12;
    bn = midiByNote ((notes)i, baseOctave);
    memset(notestoplay, sizeof(notestoplay), 0);
    MIDI.sendNoteOn(bn, 127, 1);
    
    // Create chord (notes root, intervals i, int *notes_array)
    createChord((notes)i, mode[progression].chord_type, (int*)curNote, &sh1, &sh2);

    // Create the progression
    if ((order == 0)||(order==2)||(order==3))
      for (i=0; i<steps; i++)
      {
          shift = i%3;
          oct_shift = i/3;
          if (shift == 1)
              oct_shift+=sh1;
          if (shift == 2)
              oct_shift+=sh2;
          notestoplay[notes_added] = midiByNote (curNote[shift], octaveShift+oct_shift);
          notes_added++;
      }
    if ((order == 1)||(order==2)||(order==2))
      for (i=steps; i>0; i--)
      {
          shift = i%3;
          oct_shift = i/3;
          if (shift == 1)
              oct_shift+=sh1;
          if (shift == 2)
              oct_shift+=sh2;
          notestoplay[notes_added] = midiByNote (curNote[shift], octaveShift+oct_shift);
          notes_added++;
      }

    if (order==3)
      shuffle(notestoplay, notes_added);

    for (i=0; i<notes_added; i++)
    {
        MIDI.sendNoteOn(notestoplay[i], 127, 1);
        // Delay
        delay(indelay);
        // Stop note
        MIDI.sendNoteOff(notestoplay[i], 0, 1);     // Stop the note
    }

    //Stop base note
    MIDI.sendNoteOff(bn, 0, 1);
    

}

arp::arp(notes bn, short bo, short os, unsigned short st, unsigned int d, unsigned m, unsigned int p) : baseNote(bn), baseOctave(bo), octaveShift(os), steps(st), indelay(d), progression(p)
{
  order = 0;
  mode = all_chords[m];
}

arp::arp()
{
    int i;
    baseNote = D;
    baseOctave = 5;
    octaveShift = -2;
    steps = 6;
    indelay = 200;
    progression = 0;
    mode = ionian;
}
 void arp::midibegin()
 {
   MIDI.begin(4);                      // Launch MIDI
 }
