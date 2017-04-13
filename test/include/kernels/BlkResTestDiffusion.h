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
#ifndef BLKRESTESTDIFFUSION_H
#define BLKRESTESTDIFFUSION_H

#include "Kernel.h"

// Forward Declarations
class BlkResTestDiffusion;

template <>
InputParameters validParams<BlkResTestDiffusion>();

InputParameters & modifyParams(InputParameters & params);

class BlkResTestDiffusion : public Kernel
{
public:
  BlkResTestDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif // BLKRESTESTDIFFUSION_H
