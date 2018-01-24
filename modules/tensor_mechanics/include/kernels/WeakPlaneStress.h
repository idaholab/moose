//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef WEAKPLANESTRESS_H
#define WEAKPLANESTRESS_H

#include "Kernel.h"

class WeakPlaneStress;
class RankFourTensor;
class RankTwoTensor;

template <>
InputParameters validParams<WeakPlaneStress>();

class WeakPlaneStress : public Kernel
{
public:
  WeakPlaneStress(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  std::string _base_name;

  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankFourTensor> & _Jacobian_mult;

  const unsigned int _direction;
};
#endif // WEAKPLANESTRESS_H
