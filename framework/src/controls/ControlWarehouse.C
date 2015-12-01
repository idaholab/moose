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

// MOOSE includes
#include "ControlWarehouse.h"


ControlWarehouse::ControlWarehouse() :
    ExecuteMooseObjectWarehouse<Control>(/*threaded=*/false)
{
}


void
ControlWarehouse::execute(const ExecFlagType & exec_flag)
{
  const std::vector<MooseSharedPointer<Control> > & objects = _execute_objects[exec_flag].getActiveObjects();
  for (std::vector<MooseSharedPointer<Control> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
    (*it)->execute();
}


void
ControlWarehouse::setup(const ExecFlagType & exec_flag)
{
  switch (exec_flag)
  {
  case EXEC_INITIAL:
    initialSetup();
    break;
  case EXEC_TIMESTEP_BEGIN:
    timestepSetup();
    break;
  case EXEC_SUBDOMAIN:
    subdomainSetup();
    break;
  case EXEC_NONLINEAR:
    jacobianSetup();
    break;
  case EXEC_LINEAR:
    residualSetup();
    break;
  default:
    break;
  }
}
