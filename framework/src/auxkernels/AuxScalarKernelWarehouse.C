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
#include "AuxScalarKernelWarehouse.h"


AuxScalarKernelWarehouse::AuxScalarKernelWarehouse() :
    ExecuteMooseObjectWarehouse<AuxScalarKernel>(/*threaded=*/true)
{
}


void
AuxScalarKernelWarehouse::initialSetup(THREAD_ID tid)
{
  // Sort the objects
  _all_objects.sort(tid);
  _execute_objects.sort(tid);

  // Call initialSetup on all objects
  MooseObjectWarehouse<AuxScalarKernel>::initialSetup(tid);
}
