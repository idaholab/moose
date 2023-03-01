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
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * SplitCHWresBase implements the residual for the chemical
 * potential in the split form of the Cahn-Hilliard
 * equation in a general way that can be templated to a scalar or
 * tensor mobility.
 */
template <typename T>
class SplitCHWResBase : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  SplitCHWResBase(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpWJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialPropertyName _mob_name;
  const MaterialProperty<T> & _mob;

  /// is the kernel used in a coupled form?
  const bool _is_coupled;

  /// int label for the chemical potential
  unsigned int _w_var;

  /// Variable value for the chemical potential
  const VariableGradient & _grad_w;

  /// derivatives of the mobility
  std::vector<const MaterialProperty<T> *> _dmobdarg;
};

template <typename T>
SplitCHWResBase<T>::SplitCHWResBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _mob_name(getParam<MaterialPropertyName>("mob_name")),
    _mob(getMaterialProperty<T>("mob_name")),
    _is_coupled(isCoupled("w")),
    _w_var(_is_coupled ? coupled("w") : _var.number()),
    _grad_w(_is_coupled ? coupledGradient("w") : _grad_u),
    _dmobdarg(_n_args)
{
  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _n_args; ++i)
    _dmobdarg[i] = &getMaterialPropertyDerivative<T>(_mob_name, i);
}

template <typename T>
Real
SplitCHWResBase<T>::computeQpResidual()
{
  return _mob[_qp] * _grad_w[_qp] * _grad_test[_i][_qp];
}

template <typename T>
Real
SplitCHWResBase<T>::computeQpJacobian()
{
  return (_is_coupled && _w_var != _var.number()) ? 0.0 : computeQpWJacobian();
}

template <typename T>
Real
SplitCHWResBase<T>::computeQpWJacobian()
{
  return _mob[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}

template <typename T>
Real
SplitCHWResBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // c Off-Diagonal Jacobian
  if (_w_var == jvar)
    return computeQpWJacobian();

  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return (*_dmobdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_w[_qp] * _grad_test[_i][_qp];
}

template <typename T>
InputParameters
SplitCHWResBase<T>::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Split formulation Cahn-Hilliard Kernel for the chemical potential variable");
  params.addParam<MaterialPropertyName>("mob_name", "mobtemp", "The mobility used with the kernel");
  params.addCoupledVar("args", "Vector of variable arguments of the mobility");
  params.deprecateCoupledVar("args", "coupled_variables", "02/27/2024");
  params.addCoupledVar(
      "w", "Coupled chemical potential (if not specified kernel variable will be used)");
  return params;
}
