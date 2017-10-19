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

#ifndef DGKERNEL_H
#define DGKERNEL_H

// local includes
#include "DGKernelBase.h"
#include "BlockRestrictable.h"
#include "TwoMaterialPropertyInterface.h"

class DGKernel;

template <>
InputParameters validParams<DGKernel>();

/**
 * The DGKernel class is responsible for calculating the residuals for various
 * physics on internal sides (edges/faces).
 */
class DGKernel : public DGKernelBase, public BlockRestrictable, public TwoMaterialPropertyInterface
{
public:
  DGKernel(const InputParameters & parameters);
};

#endif
