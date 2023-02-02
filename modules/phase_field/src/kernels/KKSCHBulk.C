//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSCHBulk.h"

registerMooseObject("PhaseFieldApp", KKSCHBulk);

InputParameters
KKSCHBulk::validParams()
{
  InputParameters params = CHBulk<Real>::validParams();
  params.addClassDescription("KKS model kernel for the Bulk Cahn-Hilliard term. This operates on "
                             "the concentration 'c' as the non-linear variable");
  params.addRequiredParam<MaterialPropertyName>("fa_name",
                                                "Base name of the free energy function "
                                                "F (f_name in the corresponding "
                                                "derivative function material)");
  params.addRequiredParam<MaterialPropertyName>("fb_name",
                                                "Base name of the free energy function "
                                                "F (f_name in the corresponding "
                                                "derivative function material)");
  params.addRequiredCoupledVar(
      "ca", "phase concentration corresponding to the non-linear variable of this kernel");
  params.addRequiredCoupledVar(
      "cb", "phase concentration corresponding to the non-linear variable of this kernel");
  params.addCoupledVar("args_a", "Vector of additional arguments to Fa");
  params.addParam<MaterialPropertyName>(
      "h_name", "h", "Base name for the switching function h(eta)"); // TODO: everywhere else this
                                                                     // is called just "h"
  return params;
}

KKSCHBulk::KKSCHBulk(const InputParameters & parameters)
  : CHBulk<Real>(parameters),
    _ca_var(coupled("ca")),
    _ca_name(coupledName("ca", 0)),
    _cb_var(coupled("cb")),
    _cb_name(coupledName("cb", 0)),
    _prop_h(getMaterialProperty<Real>("h_name")),
    _second_derivative_Fa(getMaterialPropertyDerivative<Real>("fa_name", _ca_name, _ca_name)),
    _second_derivative_Fb(getMaterialPropertyDerivative<Real>("fb_name", _cb_name, _cb_name))
{
  // reserve space for derivatives
  _second_derivatives.resize(_n_args);
  _third_derivatives.resize(_n_args);
  _third_derivatives_ca.resize(_n_args);
  _grad_args.resize(_n_args);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _n_args; ++i)
  {

    // get the second derivative material property (TODO:warn)
    _second_derivatives[i] = &getMaterialPropertyDerivative<Real>("fa_name", _ca_name, i);

    // get the third derivative material properties
    _third_derivatives[i].resize(_n_args);
    for (unsigned int j = 0; j < _n_args; ++j)
      _third_derivatives[i][j] = &getMaterialPropertyDerivative<Real>("fa_name", _ca_name, i, j);

    MooseVariable * cvar = _coupled_standard_moose_vars[i];

    // third derivative for the on-diagonal jacobian
    _third_derivatives_ca[i] =
        &getMaterialPropertyDerivative<Real>("fa_name", _ca_name, _ca_name, cvar->name());

    // get the gradient
    _grad_args[i] = &(cvar->gradSln());
  }
}

/**
 * Note that per product and chain rules:
 * \f$ \frac{d}{du_j}\left(F(u)\nabla u\right) = \nabla u \frac {dF(u)}{du}\frac{du}{du_j} +
 * F(u)\frac{d\nabla u}{du_j} \f$
 * which is:
 * \f$ \nabla u \frac {dF(u)}{du} \phi_j + F(u) \nabla \phi_j \f$
 */
RealGradient
KKSCHBulk::computeGradDFDCons(PFFunctionType type)
{
  RealGradient res = 0.0;

  switch (type)
  {
    case Residual:
      for (unsigned int i = 0; i < _n_args; ++i)
        res += (*_second_derivatives[i])[_qp] * (*_grad_args[i])[_qp];

      return res;

    case Jacobian:
      // the non linear variable is c, but the free energy only contains the
      // phase concentrations. Equation (23) in the KKS paper gives the chain-
      // rule derivative dca/dc
      /* Real dcadc = _second_derivative_Fb[_qp]
                  / (  (1.0 - _prop_h[_qp]) * _second_derivative_Fb[_qp]
                     + _prop_h[_qp]         * _second_derivative_Fa[_qp]); */
      // The (1-h)*X_b, h*X_a pairing is opposite to what the KKSPhaseConcentration kernel does!

      res = _second_derivative_Fa[_qp] * _grad_phi[_j][_qp];

      for (unsigned int i = 0; i < _n_args; ++i)
        res += (*_third_derivatives_ca[i])[_qp] * (*_grad_args[i])[_qp] * _phi[_j][_qp];

      // convergence improves if we return 0.0 here
      return 0.0; // res * dcadc;
  }

  mooseError("Invalid type passed in");
}

Real
KKSCHBulk::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  RealGradient res = (*_second_derivatives[cvar])[_qp] * _grad_phi[_j][_qp];

  for (unsigned int i = 0; i < _n_args; ++i)
    res += (*_third_derivatives[i][cvar])[_qp] * (*_grad_args[i])[_qp] * _phi[_j][_qp];

  // keeping this term seems to improve the solution.
  return res * _grad_test[_i][_qp];
}
