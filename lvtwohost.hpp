
/*
  Copyright (C) 2011 Harry van Haaren <harryhaaren@gmail.com>
  
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


#ifndef LV2_HOST
#define LV2_HOST

#include <map>
#include <cmath>
#include <vector>
#include <iostream>

// for checking inf & NAN's
#include <limits>

// Lilv: Lv2 host library, and extensions
#include <lilv/lilvmm.hpp>
#include "lv2/lv2plug.in/ns/ext/event/event.h"
#include "lv2/lv2plug.in/ns/ext/uri-map/uri-map.h"
#include "lv2/lv2plug.in/ns/ext/event/event-helpers.h"

// This class stores information about a single port
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
      
      min = -1.f;
      max = -1.f;
      def = -1.f;
    }
};

class Lv2Host
{
  public:
    Lv2Host(int samplerate);
    ~Lv2Host();
    
    void printInfo();
    
    void setParameter(int, float);
    void setParameterAbsolute(int, float);
    
    void loadPlugin(std::string);
    
    void process(float *L, int nframes);
  
  protected:
    bool initialized;
    bool portsSuitable;
    
    int nframes;
    int samplerate;
    std::string pluginString;
    
    // Lilv variables
    Lilv::World     *world;
    Lilv::Plugin    *plugin;
    Lilv::Instance  *instance;
    Lilv::Plugins   *tmpPluginsList;
    
    // Lv2 functions
    void printNode(Lilv::Node *tmp);
    void infoPlugin(Lilv::Plugin *plugin);
    void printPlugin(Lilv::Plugin *plugin);
    void listPlugins(Lilv::Plugins *pluginsList);
    void getPluginPorts(Lilv::World *world, Lilv::Plugin *plugin);
    
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
    
    float ignore;
    
    
    void         analysePorts();
    void         connectPorts(float*);
    
    void         checkSuitablility();
    void         instantiatePlugin();
    void         loadLibrary( std::string filename );
    
    std::vector<float> controlBuffer;
    
    std::vector< PortDetails* > portDetails;
    std::map<int,std::string> paramNameMap;
};

#endif
