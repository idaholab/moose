/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
