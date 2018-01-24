/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSSplitCHCRes.h"

template <>
InputParameters
validParams<KKSSplitCHCRes>()
{
  InputParameters params = validParams<SplitCHBase>();
  params.addClassDescription("KKS model kernel for the split Bulk Cahn-Hilliard term. This "
                             "operates on the chemical potential 'c' as the non-linear variable");
  params.addRequiredParam<MaterialPropertyName>(
      "fa_name",
      "Base name of the free energy function F (f_base in the corresponding KKSBaseMaterial)");
  params.addRequiredParam<MaterialPropertyName>(
      "fb_name",
      "Base name of the free energy function F (f_base in the corresponding KKSBaseMaterial)");
  params.addRequiredCoupledVar(
      "ca", "phase concentration corresponding to the non-linear variable of this kernel");
  params.addRequiredCoupledVar(
      "cb", "phase concentration corresponding to the non-linear variable of this kernel");
  params.addCoupledVar("args_a", "Vector of additional arguments to Fa");
  params.addParam<MaterialPropertyName>(
      "h_name", "h", "Base name for the switching function h(eta)"); // TODO: everywhere else this
                                                                     // is called just "h"
  params.addRequiredCoupledVar("w",
                               "Chemical potenial non-linear helper variable for the split solve");

  return params;
}

KKSSplitCHCRes::KKSSplitCHCRes(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<SplitCHBase>>(parameters),
    // number of coupled variables (ca, args_a[])
    _nvar(_coupled_moose_vars.size()),
    _ca_var(coupled("ca")),
    _ca_name(getVar("ca", 0)->name()),
    _cb_var(coupled("cb")),
    _cb_name(getVar("cb", 0)->name()),
    _prop_h(getMaterialProperty<Real>("h_name")),
    _first_derivative_Fa(getMaterialPropertyDerivative<Real>("fa_name", _ca_name)),
    _second_derivative_Fa(getMaterialPropertyDerivative<Real>("fa_name", _ca_name, _ca_name)),
    _second_derivative_Fb(getMaterialPropertyDerivative<Real>("fb_name", _cb_name, _cb_name)),
    _w_var(coupled("w")),
    _w(coupledValue("w"))
{
  // reserve space for derivatives
  _d2Fadcadarg.resize(_nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    MooseVariable * cvar = this->_coupled_moose_vars[i];

    // get the second derivative material property
    _d2Fadcadarg[i] = &getMaterialPropertyDerivative<Real>("fa_name", _ca_name, cvar->name());
  }
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
      return _first_derivative_Fa[_qp]; // dFa/dca ( = dFb/dcb = dF/dc)

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

  if (jvar == _ca_var)
    return _phi[_j][_qp] * _test[_i][_qp] * _second_derivative_Fa[_qp];

  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return _phi[_j][_qp] * _test[_i][_qp] * (*_d2Fadcadarg[cvar])[_qp];
}
