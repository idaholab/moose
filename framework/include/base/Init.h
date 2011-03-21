#ifndef INIT_H_
#define INIT_H_

#include "libmesh.h"
#include "getpot.h"

namespace Moose {

class Init: public LibMeshInit {
public:
  Init(int argc, char *argv[]);
  virtual ~Init();

protected:
};


extern GetPot *command_line;

}

#endif /* INIT_H_ */
