#define DR_WAV_IMPLEMENTATION
#include <iostream>
#include <string>

#include "dr_wav.h"

using namespace std;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("read <.wav> # open and dump from a .wav file");
    return 1;
  }

  drwav* pWav = drwav_open_file(argv[1]);
  if (pWav == nullptr) {
    return -1;
  }

  float* pSampleData = (float*)malloc((size_t)pWav->totalPCMFrameCount *
                                      pWav->channels * sizeof(float));
  drwav_read_f32(pWav, pWav->totalPCMFrameCount, pSampleData);

  drwav_close(pWav);

  if (pWav->channels == 1)
    for (unsigned i = 0; i < pWav->totalPCMFrameCount; ++i)
      printf("%f\n", pSampleData[i]);
  else if (pWav->channels == 2) {
    for (unsigned i = 0; i < pWav->totalPCMFrameCount; ++i)
      printf("%f\n", (pSampleData[2 * i] + pSampleData[2 * i + 1]) / 2);
  } else {
    printf("can't handle %d channels\n", pWav->channels);
    return 1;
  }
}
