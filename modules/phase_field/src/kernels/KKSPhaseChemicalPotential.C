/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSPhaseChemicalPotential.h"
#include "MathUtils.h"

using namespace MathUtils;

template <>
InputParameters
validParams<KKSPhaseChemicalPotential>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("KKS model kernel to enforce the pointwise equality of phase chemical "
                             "potentials  dFa/dca = dFb/dcb. The non-linear variable of this "
                             "kernel is ca.");
  params.addRequiredCoupledVar(
      "cb", "Phase b concentration"); // note that ca is u, the non-linear variable!
  params.addRequiredParam<MaterialPropertyName>("fa_name",
                                                "Base name of the free energy function "
                                                "Fa (f_name in the corresponding "
                                                "derivative function material)");
  params.addRequiredParam<MaterialPropertyName>("fb_name",
                                                "Base name of the free energy function "
                                                "Fb (f_name in the corresponding "
                                                "derivative function material)");
  params.addCoupledVar(
      "args_a",
      "Vector of further parameters to Fa (optional, to add in second cross derivatives of Fa)");
  params.addCoupledVar(
      "args_b",
      "Vector of further parameters to Fb (optional, to add in second cross derivatives of Fb)");
  return params;
}

KKSPhaseChemicalPotential::KKSPhaseChemicalPotential(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _cb_var(coupled("cb")),
    _cb_name(getVar("cb", 0)->name()),
    // first derivatives
    _dfadca(getMaterialPropertyDerivative<Real>("fa_name", _var.name())),
    _dfbdcb(getMaterialPropertyDerivative<Real>("fb_name", _cb_name)),
    // second derivatives d2F/dx*dca for jacobian diagonal elements
    _d2fadca2(getMaterialPropertyDerivative<Real>("fa_name", _var.name(), _var.name())),
    _d2fbdcbca(getMaterialPropertyDerivative<Real>("fb_name", _cb_name, _var.name()))
{
  MooseVariable * arg;
  unsigned int i;

#ifdef DEBUG
  _console << "KKSPhaseChemicalPotential(" << name() << ") " << _var.name() << ' ' << _cb_name
           << '\n';
#endif

  unsigned int nvar = _coupled_moose_vars.size();
  _d2fadcadarg.resize(nvar);
  _d2fbdcbdarg.resize(nvar);

  for (i = 0; i < nvar; ++i)
  {
    // get the moose variable
    arg = _coupled_moose_vars[i];

    // lookup table for the material properties representing the derivatives needed for the
    // off-diagonal jacobian
    _d2fadcadarg[i] = &getMaterialPropertyDerivative<Real>("fa_name", _var.name(), arg->name());
    _d2fbdcbdarg[i] = &getMaterialPropertyDerivative<Real>("fb_name", _cb_name, arg->name());
  }
}

void
KKSPhaseChemicalPotential::initialSetup()
{
  validateNonlinearCoupling<Real>("fa_name");
  validateNonlinearCoupling<Real>("fb_name");
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
  return _test[_i][_qp] * _phi[_j][_qp] * (_d2fadca2[_qp] - _d2fbdcbca[_qp]);
}

Real
KKSPhaseChemicalPotential::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return _test[_i][_qp] * _phi[_j][_qp] * ((*_d2fadcadarg[cvar])[_qp] - (*_d2fbdcbdarg[cvar])[_qp]);
}
