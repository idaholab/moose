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

#ifndef NODALAUXKERNELWAREHOUSE_H
#define NODALAUXKERNELWAREHOUSE_H

// MOOSE includes
#include "MooseObjectWarehouse.h"
#include "AuxKernel.h"

class NodalAuxKernelWarehouse : public MooseObjectWarehouse<AuxKernel>
{
public:

  /**
   * Constructor.
   */
  NodalAuxKernelWarehouse();

  /**
   * Sorts the AuxScalarKenels prior to calling the initialSetup.
   */
  virtual void initialSetup(THREAD_ID tid);
};

#endif // NODALAUXKERNELWAREHOUSE_H
