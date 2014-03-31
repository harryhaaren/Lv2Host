/*
 * Author: Harry van Haaren 2014
 *         harryhaaren@gmail.com
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LV2_HOST
#define LV2_HOST

#include <map>
#include <cmath>
#include <limits>
#include <vector>
#include <string>
#include <iostream>

// Lilv: Lv2 host library, and extensions
#include <lilv/lilv.h>
#include <lilv/lilvmm.hpp>
#include "lv2/lv2plug.in/ns/ext/event/event.h"
#include "lv2/lv2plug.in/ns/ext/uri-map/uri-map.h"
#include "lv2/lv2plug.in/ns/ext/event/event-helpers.h"

class PortDetails
{
  public:
    bool input;
    bool output;
    
    bool audio;
    bool event;
    bool control;
    bool midiEvent;
    
    bool logarithmic;
    
    float min;
    float max;
    float def;
    
    PortDetails()
    {
      input = false;
      output = false;
      
      audio = false;
      event = false;
      control = false;
      midiEvent = false;
      
      logarithmic = false;
      
      min = 0.f;
      max = 0.f;
      def = 0.f;
    }
};

class Lv2Host
{
  public:
    Lv2Host(int track, int samplerate, int audioBufferSize, std::string path, int pluginNumber = 0);
    ~Lv2Host();
    
    void printInfo();
    
    void setParameter(int, float);
    void setParameterAbsolute(int, float);
    
    void loadPlugin(std::string);
    
    void process(unsigned int nframes);
    float** getAudioBuffers();
  
  protected:
    int ID;
    static int privateID;
    
    int track;
    
    // General purpose vars
    bool pushChangesToUI;
    
    bool initialized;
    bool portsSuitable;
    
    int nframes;
    int samplerate;
    std::string pluginString;
    
    // Lilv variables
    Lilv::World*  world;
    Lilv::Plugin*  plugin;
    Lilv::Instance*  instance;
    Lilv::Plugins* tmpPluginsList;
    
    // Lv2 functions
    void printNode(Lilv::Node* tmp);
    void infoPlugin(Lilv::Plugin* plugin);
    void printPlugin(Lilv::Plugin* plugin);
    void listPlugins(Lilv::Plugins* pluginsList);
    void getPluginPorts(Lilv::World* world, Lilv::Plugin* plugin);
    
    // all port number variables
    int numPorts;
    
    int numInputs;
    int numOutputs;
    
    int numAudio;
    int numControl;
    
    int numAudioInputs;
    int numAudioOutputs;
    
    int numControlInputs;
    int numControlOutputs;
    
    int inputPortL;
    int inputPortR;
    int outputPortL;
    int outputPortR;
    int controlInputPort;
    
    int oscNframeCounter;

    int audioBufferSize;
    
    
    void analysePorts();
    
    void connectPorts();
    
    void checkSuitablility();
    void instantiatePlugin();
    void loadLibrary( std::string filename );

    // buffers to pass in / out control / audio data
    std::vector<float> controlBuffer;
    std::vector<float> controlOutputBuffer;

    float** audioBuffers;
    
    std::vector< PortDetails* > portDetails;
    
    std::map<int,std::string>paramNameMap;
};

#endif

