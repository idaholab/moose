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

#include "ControlWarehouse.h"

ControlWarehouse::ControlWarehouse() :
    WarehouseBase(/*threaded = */ false)
{
}

void
ControlWarehouse::execute(const ExecFlagType & exec_flag, THREAD_ID tid)
{
  checkThreadID(tid);
  const std::vector<MooseSharedPointer<Control> > & objects = getActive(exec_flag, tid);
  for (std::vector<MooseSharedPointer<Control> >::const_iterator it = objects.begin(); it != objects.end(); ++it)
    (*it)->execute();
}
