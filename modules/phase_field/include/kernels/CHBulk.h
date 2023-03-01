//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 * This is the Cahn-Hilliard equation base class that implements the bulk or
 * local energy term of the equation. It is templated on the type of the mobility,
 * which can be either a number (Real) or a tensor (RealValueTensor).
 * See M.R. Tonks et al. / Computational Materials Science 51 (2012) 20-29 for more information.
 * Note that the function computeGradDFDCons MUST be overridden in any kernel that inherits from
 * CHBulk. Use CHMath as an example of how this works.
 */
template <typename T>
class CHBulk : public DerivativeMaterialInterface<JvarMapKernelInterface<KernelGrad>>
{
public:
  CHBulk(const InputParameters & parameters);

  static InputParameters validParams();
  virtual void initialSetup();

protected:
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  enum PFFunctionType
  {
    Jacobian,
    Residual
  };

  virtual RealGradient computeGradDFDCons(PFFunctionType type) = 0;

  /// Mobility
  const MaterialProperty<T> & _M;

  /// Mobility derivative w.r.t. concentration
  const MaterialProperty<T> & _dMdc;

  /// Mobility derivative w.r.t coupled variables
  std::vector<const MaterialProperty<T> *> _dMdarg;
};

template <typename T>
CHBulk<T>::CHBulk(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<KernelGrad>>(parameters),
    _M(getMaterialProperty<T>("mob_name")),
    _dMdc(getMaterialPropertyDerivative<T>("mob_name", _var.name()))
{
  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dMdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _dMdarg[i] = &getMaterialPropertyDerivative<T>("mob_name", _coupled_moose_vars[i]->name());
}

template <typename T>
InputParameters
CHBulk<T>::validParams()
{
  InputParameters params = KernelGrad::validParams();
  params.addClassDescription("Cahn-Hilliard base Kernel");
  params.addParam<MaterialPropertyName>("mob_name", "M", "The mobility used with the kernel");
  params.addCoupledVar("args", "Vector of variable arguments of the mobility");
  params.deprecateCoupledVar("args", "coupled_variables", "02/27/2024");
  return params;
}

template <typename T>
void
CHBulk<T>::initialSetup()
{
  validateNonlinearCoupling<Real>("mob_name");
}

template <typename T>
RealGradient
CHBulk<T>::precomputeQpResidual()
{
  return _M[_qp] * computeGradDFDCons(Residual);
}

template <typename T>
RealGradient
CHBulk<T>::precomputeQpJacobian()
{
  RealGradient grad_value = _M[_qp] * computeGradDFDCons(Jacobian) +
                            _dMdc[_qp] * _phi[_j][_qp] * computeGradDFDCons(Residual);

  return grad_value;
}

template <typename T>
Real
CHBulk<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return (*_dMdarg[cvar])[_qp] * _phi[_j][_qp] * computeGradDFDCons(Residual) * _grad_test[_i][_qp];
}
