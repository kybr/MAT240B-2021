#define DR_WAV_IMPLEMENTATION
#include <iostream>
#include <string>

#include "dr_wav.h"

using namespace std;

int main(int argc, char* argv[]) {
  string fileName("out.wav");
  if (argc > 1) fileName = argv[1];

  string line;
  getline(cin, line);
  bool stereo = line.find(",") != string::npos;

  drwav_data_format format;
  format.channels = stereo ? 2 : 1;
  format.container = drwav_container_riff;
  format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
  format.sampleRate = 48000;
  format.bitsPerSample = 32;
  drwav* pWav = drwav_open_file_write(fileName.c_str(), &format);

  if (stereo) {
    while (line != "") {
      float f[2] = {0};
      f[0] = stof(line);
      f[1] = stof(line.substr(1 + line.find(",")));
      if (2 != drwav_write(pWav, 2, &f)) {
        cout << "FAIL" << endl;
        exit(1);
      }
      getline(std::cin, line);
    }
  } else {
    while (line != "") {
      float f = stof(line);
      if (1 != drwav_write(pWav, 1, &f)) {
        cout << "FAIL" << endl;
        exit(1);
      }
      getline(std::cin, line);
    }
  }

  drwav_close(pWav);
}
