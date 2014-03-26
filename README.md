LV2 Host C++ Class
==================

This class aims to be an implementation of an LV2 plugin host, with various
common extensions supported. It should serve as a good starting point for
developers wishing to implement LV2 hosting capabilities in thier programs.

Intro
----
This class supports all of the "core" LV2 features, audio ports, control ports,
and the discovery, intitialization, cleanup functions.

Extensions
----
This class will eventually support the following extensions:
```
URID map / unmap
Logging
Atom messages
State
MIDI
Presets
Time
Worker
```
