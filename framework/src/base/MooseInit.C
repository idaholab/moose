#include "MooseInit.h"
#include "Moose.h"
#include "ParallelUniqueId.h"

MooseInit::MooseInit(int argc, char *argv[]) :
	LibMeshInit(argc, argv)
{
  ParallelUniqueId::initialize();
  Moose::command_line = new GetPot(argc, argv);

  Moose::registerObjects();
}

MooseInit::~MooseInit()
{
  delete Moose::command_line;
}

namespace Moose
{

GetPot *command_line = NULL;

}
