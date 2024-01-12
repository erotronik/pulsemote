
// https://github.com/OpenDGLab/OpenDGLab-Connect/blob/master/src/services/DGLab.js

void coyote_mode_breath(short *waveclock, int *ax, int *ay, int *az) {
  // like the 'breathe' mode (1100mS cycle)
  *ax = 0; *ay = 0; *az = 0;
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
  if (*waveclock < 8) {
    *ax = 1;
    *ay = 9;
    *az = *waveclock * 4;
    if (*az > 20) *az = 20;
  }
  (*waveclock)++;
  if (*waveclock > (7+3)) {
    *waveclock = 0;
    //cyclecount--;
  }
}
