#include "KKSPhaseChemicalPotential.h"
#include "MathUtils.h"

using namespace MathUtils;

template<>
InputParameters validParams<KKSPhaseChemicalPotential>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("KKS model kernel to enforce the pointwise equality of phase chemical potentials  dFa/dca = dFb/dcb. The non-linear variable of this kernel is ca.");
  params.addRequiredCoupledVar("cb", "Phase b concentration"); // note that ca is u, the non-linear variable!
  params.addRequiredParam<std::string>("fa_name", "Base name of the free energy function Fa (f_name in the corresponding DerivativeBaseMaterial)");
  params.addRequiredParam<std::string>("fb_name", "Base name of the free energy function Fb (f_name in the corresponding DerivativeBaseMaterial)");
  params.addCoupledVar("args_a", "Vector of further parameters to Fa (optional, to add in second cross derivatives of Fa)");
  params.addCoupledVar("args_b", "Vector of further parameters to Fb (optional, to add in second cross derivatives of Fb)");
  return params;
}

KKSPhaseChemicalPotential::KKSPhaseChemicalPotential(const std::string & name,
                                                     InputParameters parameters) :
    DerivativeMaterialInterface<JvarMapInterface<Kernel> >(name, parameters),
    _cb_var(coupled("cb")),
    _cb_name(getVar("cb", 0)->name()),
    _Fa_name(getParam<std::string>("fa_name")),
    _Fb_name(getParam<std::string>("fb_name")),
    // first derivatives
    _dfadca(getMaterialPropertyDerivative<Real>(_Fa_name, _var.name())),
    _dfbdcb(getMaterialPropertyDerivative<Real>(_Fb_name, _cb_name)),
    // second derivatives d2F/dx*dca for jacobian diagonal elements
    _d2fadca2(getMaterialPropertyDerivative<Real>(_Fa_name, _var.name(), _var.name())),
    _d2fbdcbca(getMaterialPropertyDerivative<Real>(_Fb_name, _cb_name, _var.name()))
{
  MooseVariable *arg;
  unsigned int i;

  _console << "DEBUG " << name << ' ' << _var.name() << ' ' << _cb_name << '\n';

  unsigned int nvar = _coupled_moose_vars.size();
  _off_diag_a.resize(nvar);
  _off_diag_b.resize(nvar);

  for (i = 0; i < nvar; ++i)
  {
    // get the moose variable
    arg = _coupled_moose_vars[i];

    // lookup table for the material properties representing the derivatives needed for the off-diagonal jacobian
    _off_diag_a[i] = &getMaterialPropertyDerivative<Real>(_Fa_name, _var.name(), arg->name());
    _off_diag_b[i] = &getMaterialPropertyDerivative<Real>(_Fb_name, _cb_name, arg->name());
  }
}

Real
KKSPhaseChemicalPotential::computeQpResidual()
{
  // enforce _dfadca==_dfbdcb
  return _test[_i][_qp] * (_dfadca[_qp] - _dfbdcb[_qp]);
}

Real
KKSPhaseChemicalPotential::computeQpJacobian()
{
  // for on diagonal we return the d/dca derivative of the residual
  return _test[_i][_qp] * _phi[_j][_qp] * (_d2fadca2[_qp] - _d2fbdcbca[_qp]); // OK
}

Real
KKSPhaseChemicalPotential::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  return _test[_i][_qp] * _phi[_j][_qp] * ((*_off_diag_a[cvar])[_qp] - (*_off_diag_b[cvar])[_qp]); // This contributes to a buggy Jacobian
}
