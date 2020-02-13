//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

class WeakPlaneStress;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;
template <typename>
class RankFourTensorTempl;
typedef RankFourTensorTempl<Real> RankFourTensor;

template <>
InputParameters validParams<WeakPlaneStress>();

class WeakPlaneStress : public DerivativeMaterialInterface<Kernel>
{
public:
  static InputParameters validParams();

  WeakPlaneStress(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Base name of the material system that this kernel applies to
  const std::string _base_name;

  /// The stress tensor that provides the out-of-plane stress
  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankFourTensor> & _Jacobian_mult;

  /// The direction of the out-of-plane strain variable
  const unsigned int _direction;

  /// Coupled displacement variables
  const bool _disp_coupled;

  /// Number of displacement variables
  unsigned int _ndisp;

  /// Variable numbers of the displacement variables
  std::vector<unsigned int> _disp_var;

  const bool _temp_coupled;
  const unsigned int _temp_var;

  /// d(strain)/d(temperature), if computed by ComputeThermalExpansionEigenstrain
  const MaterialProperty<RankTwoTensor> * const _deigenstrain_dT;
};
