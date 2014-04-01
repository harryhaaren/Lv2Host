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

#include "lvtwohost.hxx"

int Lv2Host::privateID = 0;

Lv2Host::Lv2Host( int t, int samprate, int audioBufferSize, std::string path, int pluginNumber) :
  ID ( privateID++ ),
  initialized( false ),
  track( t ),
  samplerate( samprate ),
  audioBufferSize ( audioBufferSize )
{
  numPorts = 0;
  numControlInputs = 0;
  numControlOutputs= 0;
  numAudioInputs = 0;
  numAudioOutputs = 0;
  
  for (int i = 0; i < 255; i++)
  {
    controlBuffer.push_back(0.0);
    controlOutputBuffer.push_back(0.0);
  }
  
  std::string loadString = path;
  loadPlugin( path );
}

void Lv2Host::loadPlugin(std::string inPluginUriStr)
{
  std::cout << "Lv2Host::loadPlugin()!" << std::endl;
  
    // A world contains all the info about every plugin on the system
  world = new Lilv::World();
  
  std::cout << "LvTwoHost(): lilv loads world of plugins now" << std::endl;
  
  // load the entire world
  world->load_all();
  
  // then get them: so far they're only stored in memory, and now we get
  // a variable name with which we can acces / instantiate the plugins
  Lilv::Plugins tmpPlugs = (Lilv::Plugins)world->get_all_plugins();
  
  //listPlugins( &tmpPluginsList );
  
  std::string pluginUriStr = inPluginUriStr;
  
  Lilv::Node plugin_uri = world->new_uri( &pluginUriStr[0] );
  
  Lilv::Plugin p = tmpPlugs.get_by_uri( plugin_uri );
  plugin = &p;
  
  numPorts = plugin->get_num_ports();
  
  Lilv::Node tmpUriNode = plugin->get_name();
  pluginString = tmpUriNode.as_string();
  
  getPluginPorts( world, plugin );
  
  //   Create new instance
  instance = Lilv::Instance::create(p, samplerate, NULL);
  
  numPorts = (int)plugin->get_num_ports();
  
  if (!instance)
  {
    std::cout << "Instanciating of " << pluginUriStr << " failed!" << std::endl;
    initialized = false;
  }
  else
  {
    std::cout << "Instanciating of " << pluginUriStr << " OK, activating now!" << std::endl;
    instance->activate();
    initialized = true;
  }
  connectPorts();
}

void Lv2Host::listPlugins( Lilv::Plugins* pluginsList)
{
  // loop over the array of plugins, printing data for each.
  for ( LilvIter* i = pluginsList->begin(); !pluginsList->is_end(i); i = pluginsList->next(i) )
  {
    // Get a variable that denotes a plugin in the library
    Lilv::Plugin p = pluginsList->get(i);
    
    // Nodes represent information, this one the plugins name
    Lilv::Node nameNode = p.get_name();
    std::cout << nameNode.as_string() << std::endl;
    
    // this one the plugins uri
    Lilv::Node uriNode = p.get_uri();
    std::cout << uriNode.as_uri() << std::endl << std::endl;
  }
}

void Lv2Host::infoPlugin(Lilv::Plugin* plugin)
{
  // print out the info we have on this plugin, author, website etc
  printPlugin (plugin);
  
  // get features that this plugin supports, and print them
  Lilv::Nodes supportedFeatures = plugin->get_supported_features();
  
  std::cout << "Supported features: \n";
  for ( LilvIter* i = supportedFeatures.begin(); !supportedFeatures.is_end(i); i = supportedFeatures.next(i) )
  {
    Lilv::Node tmp = supportedFeatures.get(i);
    printNode(&tmp);
  }
  std::cout << std::endl;
  
  // get features that this plugin *requires* and print them
  Lilv::Nodes requiredFeatures = plugin->get_required_features();
  std::cout << "Required features: \n";
  for ( LilvIter* i = requiredFeatures.begin(); !requiredFeatures.is_end(i); i = requiredFeatures.next(i) )
  {
    Lilv::Node tmp = requiredFeatures.get(i);
    printNode (&tmp);
    
  }
  std::cout << std::endl;
  
}

void Lv2Host::getPluginPorts(Lilv::World* world, Lilv::Plugin* plugin)
{
  // here we iter over each port that's associated with the plugin,
  // and print out that ports info
  
  //create some nodes that represent certain features: Input / Output, Audio / MIDI
  Lilv::Node audio    = world->new_uri(LILV_URI_AUDIO_PORT);
  Lilv::Node control    = world->new_uri(LILV_URI_CONTROL_PORT);
  Lilv::Node event    = world->new_uri(LILV_URI_EVENT_PORT);
  Lilv::Node input    = world->new_uri(LILV_URI_INPUT_PORT);
  Lilv::Node midiEvent  = world->new_uri(LILV_URI_MIDI_EVENT);
  Lilv::Node output    = world->new_uri(LILV_URI_OUTPUT_PORT);
  Lilv::Node logarithmic  = world->new_uri("http://lv2plug.in/ns/dev/extportinfo#logarithmic");
  
  //std::cout << "Port properties:" << std::endl;
  
  
  // get port min & max & default values
  std::vector<float>minArray(numPorts, 0.0);
  std::vector<float>maxArray(numPorts, 0.0);
  std::vector<float>defArray(numPorts, 0.0);
  
  plugin->get_port_ranges_float(  &minArray[0], &maxArray[0],&defArray[0]);
  

  audioBuffers = new float*[numPorts];
  
  for (int i = 0; i < numPorts; i++)
  {
    Lilv::Port port = plugin->get_port_by_index(i);
    
    Lilv::Nodes portNodes = port.get_properties();
    
    Lilv::Node portName = port.get_name();
    Lilv::Node portSymb = port.get_symbol();
    
    // port ID, and name
    std::cout << "\tPort Num:" << i << "\tName: " << portName.as_string() << "\tType:   ";
    
    // portDetails vector, one instance of class per port, use that in the process callback
    portDetails.push_back(  new PortDetails() );
    
    if ( port.is_a( audio ) ) { std::cout << "Audio ";      portDetails.at(portDetails.size()-1)->audio    = true; }
    if ( port.is_a( event ) ) { std::cout << "Event ";      portDetails.at(portDetails.size()-1)->event    = true; }
    if ( port.is_a(midiEvent)){ std::cout << "Midi event "; portDetails.at(portDetails.size()-1)->midiEvent= true; }
    if ( port.is_a( control)) { std::cout << "Control ";    portDetails.at(portDetails.size()-1)->control  = true; }
    if ( port.is_a( input ) ) { std::cout << " input ";     portDetails.at(portDetails.size()-1)->input    = true; }
    if ( port.is_a( output )) { std::cout << " output ";    portDetails.at(portDetails.size()-1)->output   = true; }

    std::cout << std::endl;
    // store MIN MAX & DEFAULT values (got them earlier)
    portDetails.at(portDetails.size()-1)->min = minArray.at(i);
    portDetails.at(portDetails.size()-1)->max = maxArray.at(i);
    portDetails.at(portDetails.size()-1)->def = defArray.at(i);
    
    if ( port.has_property( logarithmic ) )
    {
      portDetails.at(portDetails.size()-1)->logarithmic = true;
    }

    controlBuffer.push_back(0.0);
    controlOutputBuffer.push_back(0.0);
    if ( port.is_a(control) && port.is_a( input ) )
    {
      // push the "i" value, and its corresponding name onto a map,
      // to be retrived later when changing parameters
      paramNameMap.insert( std::pair<int, std::string>(i, portName.as_string()));
      // keep track of how many ctrl-ins we have
      numControlInputs++;
      setParameter(i, portDetails.at(i)->def);

    } else if (port.is_a(control) && port.is_a(input)) {
        paramNameMap.insert(std::pair<int, std::string>(i, "not a control input port"));
        numAudioInputs++;
    } else if (port.is_a(control) && port.is_a(output)) {
        paramNameMap.insert(std::pair<int, std::string>(i, "not a control input port"));
        numAudioOutputs++;
    } else {
        paramNameMap.insert(std::pair<int, std::string>(i, "not a control input port"));
    }
    
    int counter = 0;
    for ( LilvIter* i = portNodes.begin(); !portNodes.is_end(i); i = portNodes.next(i), counter++ )
    {
      Lilv::Node node = portNodes.get(i);
      //std::cout << "\t\tProperty"  << counter << ": ";
      //printNode( &node );
    }
  }
  
  std::cout << "Lv2Host::getPluginPorts() DONE!" << std::endl;
}

void Lv2Host::printPlugin(Lilv::Plugin* plugin)
{
  std::cout << "Plugin info: " << std::endl;
  std::cout << "Name: " << plugin->get_name().as_string() << std::endl;
  std::cout << "URI: " << plugin->get_uri().as_string() << std::endl;
  
  if ( plugin->get_author_name().is_string() )
    std::cout << "Plugin by " << plugin->get_author_name().as_string()    << ""    << std::endl;
  
  if ( plugin->get_author_name().is_string() )
    std::cout << "Email     " << plugin->get_author_email().as_string()    << ""    << std::endl;
  
  if ( plugin->get_author_name().is_string() )
    std::cout << "Homepage  " << plugin->get_author_homepage().as_string()  << ""    << std::endl;
  
  std::cout << "Plugin has " << plugin->get_num_ports()          << " ports"  << std::endl;
  
  std::cout << std::endl;
}

void Lv2Host::printNode(Lilv::Node* tmp)
{
  // helper function to print a node's contents
  if ( tmp->is_string() )
    std::cout << "String: " << tmp->as_string() << std::endl;
  
  if ( tmp->is_uri() )
    std::cout << "URI: " << tmp->as_uri() << std::endl;
  
  if ( tmp->is_float() )
    std::cout << "float: " << tmp->as_float() << std::endl;
  
  if ( tmp->is_int() )
    std::cout << "int: " << tmp->as_int() << std::endl;
}

void Lv2Host::printInfo()
{
  std::cout << "Lv2Host plugin:" << pluginString << " Init: " <<  initialized;
}

/*
void Lv2Host::subscribeFunction()
{
}
*/

void Lv2Host::setParameter(int param, float value)
{
  std::string name;
  
  if (param >= 0 && param < numPorts ) // range check the parameter
  {
    if (portDetails.at(param)->control && portDetails.at(param)->input) {
        std::map<int,std::string>::iterator iter = paramNameMap.find(param);
    
        if (iter != paramNameMap.end() ) {
            name = iter->second;
        } else {
            name = "invalid value";
        }

        // check for NaN
        if (value != value) {
            value = 0;
        }
        // Clamp values to min and max
        if (portDetails.at(param)->max == portDetails.at(param)->max && value > portDetails.at(param)->max) {
            value = portDetails.at(param)->max;
        } else if (portDetails.at(param)->min == portDetails.at(param)->min && value < portDetails.at(param)->min) {
            value = portDetails.at(param)->min;
        }

        std::cout << "Lv2Host::setParameter( " << name <<  ", " << ": " << value << " ) Min:" << portDetails.at(param)->min <<"; " << "Max:" << portDetails.at(param)->max << std::endl;
        controlBuffer[param] = value;
    
        // make GUI redraw
        //rh->sendOsc( Osc::PATH_TRACK_SET_PLUGIN_PARAMETER_ABSOLUTE, -2, ID, param, value );
    } else {
        std::cout << "Lv2Host::setParameter() not a control input port, param: " << param << std::endl;
    }
  } else {
    std::cout << "Lv2Host::setParameter() accesing out of bounds, param: " << param << std::endl;
  }
}

void Lv2Host::connectPorts()
{
  for (int i = 0; i < numPorts; i++)
  {
    if( portDetails.at(i)->audio && portDetails.at(i)->input )
    {
      std::cout << "Connecting Audio Input!  port #: " << i << std::endl;
      audioBuffers[i] = new float[audioBufferSize];
      instance->connect_port( i, audioBuffers[i]);
    }
    else if( portDetails.at(i)->audio && portDetails.at(i)->output )
    {
      std::cout << "Connecting Audio Output!  port #: " << i << std::endl;
      audioBuffers[i] = new float[audioBufferSize];
      instance->connect_port( i, audioBuffers[i]);
    }
    
    else if( portDetails.at(i)->control && portDetails.at(i)->input )
    {
      // std::cout << "Connecting Control Input!  port #: " << i << std::endl;
      instance->connect_port( i, &controlBuffer.at(i) );
    }
    else if( portDetails.at(i)->control && portDetails.at(i)->output )
    {
      // std::cout << "Connecting Control Output!  port #: " << i << std::endl;
      instance->connect_port( i, &controlOutputBuffer.at(i) );
    }
    else
    {
      // we don't know how to connect this port properly, its an
      // unknown type, so don't let the plugin run, as it will segfault
      // when running with an un-connected port
        std::cout << "Port could not be connected #: " << i << std::endl;
        initialized = 0;
    }
  }
}

float** Lv2Host::getAudioBuffers() {
    return audioBuffers;
}

void Lv2Host::process(unsigned int nframes)
{
  if( initialized != 1 )
  {
    std::cout << "Lv2Host::process(): Warning: Running uninitialized plugin!" << std::endl;
    return;
  }
  
  // std::cout << "Lv2Host::process(): running NOW!"  << std::endl;
  instance->run(nframes);
  return;
}

Lv2Host::~Lv2Host()
{
  std::cout << "~Lv2Host() destroyed" << std::endl;
}
