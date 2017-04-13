/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSCHBulk.h"

template <>
InputParameters
validParams<KKSCHBulk>()
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
    // number of coupled variables (ca, args_a[])
    _nvar(_coupled_moose_vars.size()),
    _ca_var(coupled("ca")),
    _ca_name(getVar("ca", 0)->name()),
    _cb_var(coupled("cb")),
    _cb_name(getVar("cb", 0)->name()),
    _prop_h(getMaterialProperty<Real>("h_name")),
    _second_derivative_Fa(getMaterialPropertyDerivative<Real>("fa_name", _ca_name, _ca_name)),
    _second_derivative_Fb(getMaterialPropertyDerivative<Real>("fb_name", _cb_name, _cb_name))
{
  // reserve space for derivatives
  _second_derivatives.resize(_nvar);
  _third_derivatives.resize(_nvar);
  _third_derivatives_ca.resize(_nvar);
  _grad_args.resize(_nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    MooseVariable * cvar = _coupled_moose_vars[i];

    // get the second derivative material property (TODO:warn)
    _second_derivatives[i] =
        &getMaterialPropertyDerivative<Real>("fa_name", _ca_name, cvar->name());

    // get the third derivative material properties
    _third_derivatives[i].resize(_nvar);
    for (unsigned int j = 0; j < _nvar; ++j)
      _third_derivatives[i][j] = &getMaterialPropertyDerivative<Real>(
          "fa_name", _ca_name, cvar->name(), _coupled_moose_vars[j]->name());

    // third derivative for the on-diagonal jacobian
    _third_derivatives_ca[i] =
        &getMaterialPropertyDerivative<Real>("fa_name", _ca_name, cvar->name(), _ca_name);

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
      for (unsigned int i = 0; i < _nvar; ++i)
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

      for (unsigned int i = 0; i < _nvar; ++i)
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

  for (unsigned int i = 0; i < _nvar; ++i)
    res += (*_third_derivatives[i][cvar])[_qp] * (*_grad_args[i])[_qp] * _phi[_j][_qp];

  // keeping this term seems to improve the solution.
  return res * _grad_test[_j][_qp];
}
