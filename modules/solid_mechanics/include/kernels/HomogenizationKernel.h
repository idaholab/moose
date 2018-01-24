/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HOMOGENIZATIONKERNEL_H
#define HOMOGENIZATIONKERNEL_H

#include "Kernel.h"

// Forward Declarations
class ColumnMajorMatrix;
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
