/*
  Copyright (C) 2014 Harry van Haaren <harryhaaren@gmail.com>
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "lvtwohost.hxx"

#include <sndfile.hh>
#include <string>

int main(int argc, char** argv)
{
  float* audioIn = new float[44100 * 5];
  float* audioOut = new float[44100 * 10];

  SndfileHandle outfile( "lvtwohostTest.wav" , SFM_WRITE,  SF_FORMAT_WAV | SF_FORMAT_FLOAT , 2 , 44100);

  // instantiate LV2 host class
  std::string pluginURI = "http://www.openavproductions.com/artyfx#satma";
  Lv2Host* host = new Lv2Host( 0, 44100, 1024, pluginURI );

  // process buffer size
  int nframes = 128;

  SndfileHandle infile("test.wav");
  infile.read(audioIn, 44100 * 10);
  
  float** audioBuffers = host->getAudioBuffers();

  for (int i = 0; i < ceil(44100 * 5 / 128); i++) {
      // copy input
      std::copy(&audioIn[i * 128], &audioIn[i * 128] + 128, audioBuffers[0]);
      host->process(nframes);
      // copy output
      for (int x = 0; x < 128; x++) {
          audioOut[i * 256 + 2 * x] = audioBuffers[1][x];
          audioOut[i * 256 + 1 + 2 * x] = audioBuffers[1][x];
      }
  }

  // save audio to disk
  outfile.write(audioOut, 44100 * 10);
  
  // cleanup
  delete host;
  delete[] audioIn;
  return 0;
}
