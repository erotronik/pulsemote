
// https://github.com/OpenDGLab/OpenDGLab-Connect/blob/master/src/services/DGLab.js

#include "coyote.h"

coyote_pattern coyote_mode_breath(short &waveclock) {
  // like the 'breathe' mode (1100mS cycle)
  coyote_pattern out;
  //instead of settting up an array like this, let's write it as a formula
  //Int8Array(3) [33, 1, 0]  0: 1 9 0
  //Int8Array(3) [33, 1, 2]  1: 1 9 4
  //Int8Array(3) [33, 1, 4]  2: 1 9 8
  //Int8Array(3) [33, 1, 6]  3: 1 9 12
  //Int8Array(3) [33, 1, 8]  4: 1 9 16
  //Int8Array(3) [33, 1, 10] 5: 1 9 20
  //Int8Array(3) [33, 1, 10] 6: 1 9 20
  //Int8Array(3) [33, 1, 10] 7: 1 9 20
  //Int8Array(3) [0, 0, 0]   8: 0 0 0
  //Int8Array(3) [0, 0, 0]   9: 0 0 0
  //Int8Array(3) [0, 0, 0]  10: 0 0 0
  if (waveclock < 8) {
    out.pulse_length = 1;
    out.pause_length = 9;
    out.amplitude = waveclock * 4;
    if (out.amplitude > 20) out.amplitude = 20;
  }
  (waveclock)++;
  if (waveclock > (7+3)) {
    waveclock = 0;
    //cyclecount--;
  }
  return out;
}
