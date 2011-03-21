#ifndef MOOSEINIT_H
#define MOOSEINIT_H

#include "libmesh.h"
#include "getpot.h"

class MooseInit : public LibMeshInit
{
public:
  MooseInit(int argc, char *argv[]);
  virtual ~MooseInit();

protected:
};

namespace Moose
{

extern GetPot *command_line;

} // namespace Moose

#endif /* INIT_H_ */
