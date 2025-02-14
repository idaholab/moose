//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelGrad.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * This class template implements a diffusion kernel with a mobility that can vary
 * spatially and can depend on variables in the simulation. Two classes are derived from
 * this template, MatDiffusion for isotropic diffusion and MatAnisoDiffusion for
 * anisotropic diffusion.
 *
 * \tparam T Type of the diffusion coefficient parameter. This can be Real for
 *           isotropic diffusion or RealTensorValue for the general anisotropic case.
 */
template <typename T>
class MatDiffusionBase : public DerivativeMaterialInterface<JvarMapKernelInterface<KernelGrad>>
{
public:
  static InputParameters validParams();

  MatDiffusionBase(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual RealGradient precomputeQpResidual() override;
  virtual RealGradient precomputeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  virtual RealGradient computeQpCJacobian();

  /// diffusion coefficient
  const MaterialProperty<T> & _diffusivity;

  /// diffusion coefficient derivative w.r.t. the kernel variable
  const MaterialProperty<T> & _ddiffusivity_dc;

  /// diffusion coefficient derivatives w.r.t. coupled variables
  std::vector<const MaterialProperty<T> *> _ddiffusivity_darg;

  /// is the kernel used in a coupled form?
  const bool _is_coupled;

  /// int label for the Concentration
  unsigned int _v_var;

  /// Gradient of the concentration
  const VariableGradient & _grad_v;
};

template <typename T>
InputParameters
MatDiffusionBase<T>::validParams()
{
  InputParameters params = KernelGrad::validParams();

  params.addParam<MaterialPropertyName>(
      "diffusivity", "D", "The diffusivity value or material property");
  params.addCoupledVar("args",
                       "Optional vector of arguments for the diffusivity. If provided and "
                       "diffusivity is a derivative parsed material, Jacobian contributions from "
                       "the diffusivity will be automatically computed");
  params.addCoupledVar("v",
                       "Coupled concentration variable for kernel to operate on; if this "
                       "is not specified, the kernel's nonlinear variable will be used as "
                       "usual");
  return params;
}

template <typename T>
MatDiffusionBase<T>::MatDiffusionBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<KernelGrad>>(parameters),
    _diffusivity(getMaterialProperty<T>("diffusivity")),
    _ddiffusivity_dc(getMaterialPropertyDerivative<T>("diffusivity", _var.name())),
    _ddiffusivity_darg(_coupled_moose_vars.size()),
    _is_coupled(isCoupled("v")),
    _v_var(_is_coupled ? coupled("v") : _var.number()),
    _grad_v(_is_coupled ? coupledGradient("v") : _grad_u)
{
  // fetch derivatives
  for (unsigned int i = 0; i < _ddiffusivity_darg.size(); ++i)
    _ddiffusivity_darg[i] =
        &getMaterialPropertyDerivative<T>("diffusivity", _coupled_moose_vars[i]->name());
}

template <typename T>
void
MatDiffusionBase<T>::initialSetup()
{
  validateNonlinearCoupling<Real>("diffusivity");
}

template <typename T>
RealGradient
MatDiffusionBase<T>::precomputeQpResidual()
{
  return _diffusivity[_qp] * _grad_v[_qp];
}

template <typename T>
RealGradient
MatDiffusionBase<T>::precomputeQpJacobian()
{
  RealGradient sum = _phi[_j][_qp] * _ddiffusivity_dc[_qp] * _grad_v[_qp];
  if (!_is_coupled)
    sum += computeQpCJacobian();

  return sum;
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  auto sum = (*_ddiffusivity_darg[cvar])[_qp] * _phi[_j][_qp] * _grad_v[_qp] * _grad_test[_i][_qp];
  if (_v_var == jvar)
    sum += computeQpCJacobian() * _grad_test[_i][_qp];

  return sum;
}

template <typename T>
RealGradient
MatDiffusionBase<T>::computeQpCJacobian()
{
  return _diffusivity[_qp] * _grad_phi[_j][_qp];
}
