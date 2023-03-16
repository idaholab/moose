//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSSplitCHCRes.h"

registerMooseObject("PhaseFieldApp", KKSSplitCHCRes);

InputParameters
KKSSplitCHCRes::validParams()
{
  InputParameters params = SplitCHBase::validParams();
  params.addClassDescription(
      "KKS model kernel for the split Bulk Cahn-Hilliard term. This kernel operates on the "
      "physical concentration 'c' as the non-linear variable");
  params.addRequiredParam<MaterialPropertyName>(
      "fa_name",
      "Base name of an arbitrary phase free energy function F (f_base in the corresponding "
      "KKSBaseMaterial)");
  params.addRequiredCoupledVar(
      "ca", "phase concentration corresponding to the non-linear variable of this kernel");
  params.addCoupledVar("args_a", "Vector of additional arguments to Fa");
  params.addRequiredCoupledVar("w",
                               "Chemical potential non-linear helper variable for the split solve");
  return params;
}

KKSSplitCHCRes::KKSSplitCHCRes(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<SplitCHBase>>(parameters),
    _ca_var(coupled("ca")),
    _ca_name(coupledName("ca", 0)),
    _dFadca(getMaterialPropertyDerivative<Real>("fa_name", _ca_name)),
    _d2Fadcadarg(_n_args),
    _w_var(coupled("w")),
    _w(coupledValue("w"))
{
  // get the second derivative material property
  for (unsigned int i = 0; i < _n_args; ++i)
    _d2Fadcadarg[i] = &getMaterialPropertyDerivative<Real>("fa_name", _ca_name, i);
}

void
KKSSplitCHCRes::initialSetup()
{
  validateNonlinearCoupling<Real>("fa_name");
  validateDerivativeMaterialPropertyBase<Real>("fa_name");
}

Real
KKSSplitCHCRes::computeQpResidual()
{
  Real residual = SplitCHBase::computeQpResidual();
  residual += -_w[_qp] * _test[_i][_qp];

  return residual;
}

/**
 * Note that per product and chain rules:
 * \f$ \frac{d}{du_j}\left(F(u)\nabla u\right) = \nabla u \frac {dF(u)}{du}\frac{du}{du_j} +
 * F(u)\frac{d\nabla u}{du_j} \f$
 * which is:
 * \f$ \nabla u \frac {dF(u)}{du} \phi_j + F(u) \nabla \phi_j \f$
 */
Real
KKSSplitCHCRes::computeDFDC(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return _dFadca[_qp]; // dFa/dca ( = dFb/dcb = dF/dc)

    case Jacobian:
      return 0.0;
  }

  mooseError("Invalid type passed in");
}

Real
KKSSplitCHCRes::computeQpOffDiagJacobian(unsigned int jvar)
{
  // treat w variable explicitly
  if (jvar == _w_var)
    return -_phi[_j][_qp] * _test[_i][_qp];

  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);
  return _phi[_j][_qp] * _test[_i][_qp] * (*_d2Fadcadarg[cvar])[_qp];
}
