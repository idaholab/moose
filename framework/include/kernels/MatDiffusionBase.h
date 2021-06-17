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
 * This class template implements a diffusion kernel with a mobility that can vary
 * spatially and can depend on variables in the simulation. Two classes are derived from
 * this template, MatDiffusion for isotropic diffusion and MatAnisoDiffusion for
 * anisotropic diffusion.
 *
 * \tparam T Type of the diffusion coefficient parameter. This can be Real for
 *           isotropic diffusion or RealTensorValue for the general anisotropic case.
 */
template <typename T>
class MatDiffusionBase : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  MatDiffusionBase(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
  virtual Real computeQpCJacobian();

  /// diffusion coefficient
  const MaterialProperty<T> & _D;

  /// diffusion coefficient derivative w.r.t. the kernel variable
  const MaterialProperty<T> & _dDdc;

  /// diffusion coefficient derivatives w.r.t. coupled variables
  std::vector<const MaterialProperty<T> *> _dDdarg;

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
  InputParameters params = Kernel::validParams();
  params.addDeprecatedParam<MaterialPropertyName>(
      "D_name",
      "The name of the diffusivity",
      "This parameter has been renamed to 'diffusivity', which is more mnemonic and more conducive "
      "to passing a number literal");
  params.addParam<MaterialPropertyName>(
      "diffusivity", "D", "The diffusivity value or material property");
  params.addCoupledVar("args",
                       "Optional vector of arguments for the diffusivity. If provided and "
                       "diffusivity is a derivative parsed material, Jacobian contributions from "
                       "the diffusivity will be automatically computed");
  params.addCoupledVar("conc", "Deprecated! Use 'v' instead");
  params.addCoupledVar("v",
                       "Coupled concentration variable for kernel to operate on; if this "
                       "is not specified, the kernel's nonlinear variable will be used as "
                       "usual");
  return params;
}

template <typename T>
MatDiffusionBase<T>::MatDiffusionBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _D(isParamValid("D_name") ? getMaterialProperty<T>("D_name")
                              : getMaterialProperty<T>("diffusivity")),
    _dDdc(getMaterialPropertyDerivative<T>(isParamValid("D_name") ? "D_name" : "diffusivity",
                                           _var.name())),
    _dDdarg(_coupled_moose_vars.size()),
    _is_coupled(isCoupled("v")),
    _v_var(_is_coupled ? coupled("v") : (isCoupled("conc") ? coupled("conc") : _var.number())),
    _grad_v(_is_coupled ? coupledGradient("v")
                        : (isCoupled("conc") ? coupledGradient("conc") : _grad_u))
{
  // deprecated variable parameter conc
  if (isCoupled("conc"))
    mooseDeprecated("In '", name(), "' the parameter 'conc' is deprecated, please use 'v' instead");

  // fetch derivatives
  for (unsigned int i = 0; i < _dDdarg.size(); ++i)
    _dDdarg[i] = &getMaterialPropertyDerivative<T>(
        isParamValid("D_name") ? "D_name" : "diffusivity", _coupled_moose_vars[i]->name());
}

template <typename T>
void
MatDiffusionBase<T>::initialSetup()
{
  validateNonlinearCoupling<Real>(parameters().isParamSetByUser("D_name") ? "D_name"
                                                                          : "diffusivity");
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpResidual()
{
  return _D[_qp] * _grad_v[_qp] * _grad_test[_i][_qp];
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpJacobian()
{
  Real sum = _phi[_j][_qp] * _dDdc[_qp] * _grad_v[_qp] * _grad_test[_i][_qp];
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

  Real sum = (*_dDdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_v[_qp] * _grad_test[_i][_qp];
  if (_v_var == jvar)
    sum += computeQpCJacobian();

  return sum;
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpCJacobian()
{
  return _D[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
