//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef HOMOGENIZATIONKERNEL_H
#define HOMOGENIZATIONKERNEL_H

#include "Kernel.h"

// Forward Declarations
class HomogenizationKernel;
class SymmElasticityTensor;
class SymmTensor;

template <>
InputParameters validParams<HomogenizationKernel>();

class HomogenizationKernel : public Kernel
{
public:
  HomogenizationKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const MaterialProperty<SymmElasticityTensor> & _elasticity_tensor;

private:
  const unsigned int _component;
  const unsigned int _column;
};
#endif // HOMOGENIZATIONKERNEL_H
