/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseInit.h"
#include "Moose.h"
#include "ParallelUniqueId.h"
#include "Factory.h"
#include "ActionFactory.h"

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
  Factory::release();
  ActionFactory::release();
}

namespace Moose
{

GetPot *command_line = NULL;

}
