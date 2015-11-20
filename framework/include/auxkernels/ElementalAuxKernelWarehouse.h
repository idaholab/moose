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

#ifndef ELEMENTALAUXKERNELWAREHOUSE_H
#define ELEMENTALAUXKERNELWAREHOUSE_H

// MOOSE includes
#include "MooseObjectWarehouse.h"
#include "AuxKernel.h"

class ElementalAuxKernelWarehouse : public MooseObjectWarehouse<AuxKernel>
{
public:

  /**
   * Constructor.
   */
  ElementalAuxKernelWarehouse();

  /**
   * Sorts the AuxScalarKenels prior to calling the initialSetup.
   */
  virtual void initialSetup(THREAD_ID tid);
};

#endif // ELEMENTALAUXSCALARKERNELWAREHOUSE_H
