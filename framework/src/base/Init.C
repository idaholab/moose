#include "Init.h"
#include "Moose.h"
#include "ParallelUniqueId.h"

Init::Init(int argc, char *argv[]) :
	LibMeshInit(argc, argv)
{
  ParallelUniqueId::initialize();
  Moose::command_line = new GetPot(argc, argv);

  Moose::registerObjects();
}

Init::~Init()
{
  delete Moose::command_line;
}

namespace Moose {

  GetPot *command_line = NULL;

}
