//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STRESSDIVERGENCERSPHERICAL_H
#define STRESSDIVERGENCERSPHERICAL_H

#include "Kernel.h"

// Forward Declarations
class SymmElasticityTensor;
class SymmTensor;

class StressDivergenceRSpherical : public Kernel
{
public:
  StressDivergenceRSpherical(const InputParameters & parameters);
  virtual ~StressDivergenceRSpherical() {}

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<SymmTensor> & _stress;
  const MaterialProperty<SymmElasticityTensor> & _Jacobian_mult;
  const MaterialProperty<SymmTensor> & _d_stress_dT;

private:
  const unsigned int _component;
  const bool _temp_coupled;
  const unsigned int _temp_var;
};

template <>
InputParameters validParams<StressDivergenceRSpherical>();

#endif // STRESSDIVERGENCERSPHERICAL_H
