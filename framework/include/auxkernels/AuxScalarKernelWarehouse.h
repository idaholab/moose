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

#ifndef AUXSCALARKERNELWAREHOUSE_H
#define AUXSCALARKERNELWAREHOUSE_H

// MOOSE includes
#include "MooseObjectWarehouse.h"
#include "AuxScalarKernel.h"

class AuxScalarKernelWarehouse : public MooseObjectWarehouse<AuxScalarKernel>
{
public:

  /**
   * Constructor.
   */
  AuxScalarKernelWarehouse();

  /**
   * Destructor.
   */
  ~AuxScalarKernelWarehouse(){}

  /**
   * Sorts the AuxScalarKenels prior to calling the initialSetup.
   */
  virtual void initialSetup(THREAD_ID tid);

};

#endif // AUXSCALARKERNELWAREHOUSE_H
