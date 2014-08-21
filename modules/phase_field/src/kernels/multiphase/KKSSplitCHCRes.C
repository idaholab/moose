#include "KKSSplitCHCRes.h"

template<>
InputParameters validParams<KKSSplitCHCRes>()
{
  InputParameters params = validParams<SplitCHBase>();
  params.addClassDescription("KKS model kernel for the split Bulk Cahn-Hilliard term. This operates on the chemical potential 'c' as the non-linear variable");
  params.addRequiredParam<std::string>("fa_name", "Base name of the free energy function F (f_base in the corresponding KKSBaseMaterial)");
  params.addRequiredParam<std::string>("fb_name", "Base name of the free energy function F (f_base in the corresponding KKSBaseMaterial)");
  params.addRequiredCoupledVar("ca", "phase concentration corresponding to the non-linear variable of this kernel");
  params.addRequiredCoupledVar("cb", "phase concentration corresponding to the non-linear variable of this kernel");
  params.addCoupledVar("args_a", "Vector of additional arguments to Fa");
  params.addParam<std::string>("h_name", "h", "Base name for the switching function h(eta)");
  params.addRequiredCoupledVar("w", "Chemical potenial non-linear helper variable for the split solve");

  return params;
}

KKSSplitCHCRes::KKSSplitCHCRes(const std::string & name, InputParameters parameters) :
    DerivativeMaterialInterface<SplitCHBase>(name, parameters),
    // number of coupled variables (ca, args_a[])
    _nvar(_coupled_moose_vars.size()),
    _Fa_name(getParam<std::string>("fa_name")),
    _Fb_name(getParam<std::string>("fb_name")),
    _h_name(getParam<std::string>("h_name")),
    _ca_var(coupled("ca")),
    _ca_name(getVar("ca", 0)->name()),
    _cb_var(coupled("cb")),
    _cb_name(getVar("cb", 0)->name()),
    _prop_h(getMaterialProperty<Real>(_h_name)),
    _first_derivative_Fa(getDerivative<Real>(_Fa_name, _ca_name)),
    _second_derivative_Fa(getDerivative<Real>(_Fa_name, _ca_name, _ca_name)),
    _second_derivative_Fb(getDerivative<Real>(_Fb_name, _cb_name, _cb_name)),
    _w_var(coupled("w")),
    _w(coupledValue("w"))
{
  // reserve space for derivatives
  _second_derivatives.resize(_nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
  {
    MooseVariable *cvar = this->_coupled_moose_vars[i];

    // get the second derivative material property (TODO:warn)
    _second_derivatives[i] = &getDerivative<Real>(_Fa_name, _ca_name, cvar->name());
  }
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
 * \f$ \frac{d}{du_j}\left(F(u)\nabla u\right) = \nabla u \frac {dF(u)}{du}\frac{du}{du_j} + F(u)\frac{d\nabla u}{du_j} \f$
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
      return 0.0; // PetSc finite differencing says this should be 0.0!
#if 0
      // the non linear variable is c, but the free energy only contains the
      // phase concentrations. Equation (23) in the KKS paper gives the chain-
      // rule derivative dca/dc
      Real dcadc = _second_derivative_Fb[_qp]
                  / (  (1.0 - _prop_h[_qp]) * _second_derivative_Fb[_qp]
                     + _prop_h[_qp]         * _second_derivative_Fa[_qp]);
      // The (1-h)*X_b, h*X_a pairing is opposite to what the KKSPhaseConcentration kernel does!

      res = _second_derivative_Fa[_qp] * _phi[_j][_qp];

      return res * dcadc + 1e9;
#endif
  }

  mooseError("Invalid type passed in");
}

Real
KKSSplitCHCRes::computeQpOffDiagJacobian(unsigned int jvar)
{
  // treat w variable explicitly
  if (jvar == _w_var)
    return -_phi[_j][_qp] * _test[_i][_qp]; // OK

  if (jvar == _ca_var)
    return _phi[_j][_qp] * _test[_i][_qp] * _second_derivative_Fa[_qp]; // OK

  if (jvar == _cb_var)
    // return _phi[_j][_qp] * _test[_i][_qp] * _second_derivative_Fb[_qp]; // PetSc fd wants this to be 0.0. Why?
    return 0.0;

  return 0.0; // OK
}
