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
  float* audio = new float[44100 * 10];

  SndfileHandle outfile( "lvtwohostTest.wav" , SFM_WRITE,  SF_FORMAT_WAV | SF_FORMAT_FLOAT , 1 , 44100);

  // instantiate LV2 host class
  std::string pluginURI = "http://www.openavproductions.com/artyfx#roomy";
  Lv2Host* host = new Lv2Host( 0, 44100, pluginURI );

  // process buffer size
  int nframes = 128;

  SndfileHandle infile("test.wav");
  infile.read(audio, 44100 * 10);

  float* audioBuffer = new float[nframes];
  for (int i = 0; i < ceil(44100 * 10 / 128); i++) {
        for (int x = 0; x < 128; x++) {
            audioBuffer[x] = audio[i * 128 + x];
        }
        host->process(nframes, audioBuffer);
        // copy output
        for (int x = 0; x < 128; x++) {
            audio[i * 128 + x] = audioBuffer[x];
        }
  }
  
  // save audio to disk
  outfile.write(audio, 44100 * 10);
  
  // cleanup
  delete host;
  delete[] audio;
  delete[] audioBuffer;
  return 0;
}
