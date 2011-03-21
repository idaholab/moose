#include "Init.h"
#include "Moose.h"
#include "ParallelUniqueId.h"

namespace Moose {

Init::Init(int argc, char *argv[]) :
	LibMeshInit(argc, argv)
{
  ParallelUniqueId::initialize();
  command_line = new GetPot(argc, argv);

  registerObjects();
}

Init::~Init()
{
  delete command_line;
}


GetPot *command_line = NULL;

} // namespace
