#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <sstream>

#include "frozen.h"
#include "libgallocy.h"


// XXX: I'm not sure why instantiating the class like this and passing it to
// (viz.) the gallocy::string type works, but passing the SingletonHeapType
// class directly does not.
class STLHeap :
  // XXX: Move anyThreadsCreated from
  // libgallocy.cpp into a stand alone module that
  // both gallocy-core and gallocy-net can depend
  // on. If we move this into SingletonHeapType
  // then everyone needs to depend on gallocy-core
  // for the single variable.
  public HL::LockedHeap<
    HL::SpinLockType,
    HL::SingletonHeap > {};


typedef gallocy::vector<gallocy::string> peer_list_t;

typedef gallocy::map<int, int> foo;


gallocy::string read_file(const char*);
gallocy::string get_me(void);
peer_list_t get_peers(void);
