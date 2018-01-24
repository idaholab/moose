//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATDIFFUSIONBASE_H
#define MATDIFFUSIONBASE_H

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
  MatDiffusionBase(const InputParameters & parameters);

  virtual void initialSetup();

  /// in class templates this function has to be a static member
  static InputParameters validParams();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
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
  unsigned int _conc_var;

  /// Gradient of the concentration
  const VariableGradient & _grad_conc;
};

template <typename T>
InputParameters
MatDiffusionBase<T>::validParams()
{
  InputParameters params = ::validParams<Kernel>();
  params.addParam<MaterialPropertyName>("D_name", "D", "The name of the diffusivity");
  params.addCoupledVar("args", "Vector of arguments of the diffusivity");
  params.addCoupledVar("conc",
                       "Coupled concentration variable for kernel to operate on; if this "
                       "is not specified, the kernel's nonlinear variable will be used as "
                       "usual");
  return params;
}

template <typename T>
MatDiffusionBase<T>::MatDiffusionBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _D(getMaterialProperty<T>("D_name")),
    _dDdc(getMaterialPropertyDerivative<T>("D_name", _var.name())),
    _dDdarg(_coupled_moose_vars.size()),
    _is_coupled(isCoupled("conc")),
    _conc_var(_is_coupled ? coupled("conc") : _var.number()),
    _grad_conc(_is_coupled ? coupledGradient("conc") : _grad_u)
{
  // fetch derivatives
  for (unsigned int i = 0; i < _dDdarg.size(); ++i)
    _dDdarg[i] = &getMaterialPropertyDerivative<T>("D_name", _coupled_moose_vars[i]->name());
}

template <typename T>
void
MatDiffusionBase<T>::initialSetup()
{
  validateNonlinearCoupling<Real>("D_name");
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpResidual()
{
  return _D[_qp] * _grad_conc[_qp] * _grad_test[_i][_qp];
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpJacobian()
{
  Real sum = _phi[_j][_qp] * _dDdc[_qp] * _grad_conc[_qp] * _grad_test[_i][_qp];
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

  Real sum = (*_dDdarg[cvar])[_qp] * _phi[_j][_qp] * _grad_conc[_qp] * _grad_test[_i][_qp];
  if (_conc_var == jvar)
    sum += computeQpCJacobian();

  return sum;
}

template <typename T>
Real
MatDiffusionBase<T>::computeQpCJacobian()
{
  return _D[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
#endif // MATDIFFUSIONBASE_H
