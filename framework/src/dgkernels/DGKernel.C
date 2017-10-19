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

#include "DGKernel.h"

template <>
InputParameters
validParams<DGKernel>()
{
  InputParameters params = validParams<DGKernelBase>();
  params += validParams<BlockRestrictable>();
  params += validParams<TwoMaterialPropertyInterface>();
  params.registerBase("DGKernel");
  return params;
}

DGKernel::DGKernel(const InputParameters & parameters)
  : DGKernelBase(parameters),
    BlockRestrictable(this),
    TwoMaterialPropertyInterface(this, blockIDs(), boundaryIDs())
{
}
