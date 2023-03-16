//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSPhaseChemicalPotential.h"
#include "MathUtils.h"

using namespace MathUtils;

registerMooseObject("PhaseFieldApp", KKSPhaseChemicalPotential);

InputParameters
KKSPhaseChemicalPotential::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("KKS model kernel to enforce the pointwise equality of phase chemical "
                             "potentials $dF_a/dc_a = dF_b/dc_b$. The non-linear variable of this "
                             "kernel is $c_a$.");
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
  params.addParam<Real>("ka",
                        1.0,
                        "Site fraction for the ca variable (specify this if ca is a sublattice "
                        "concentration, and make sure it is a true site fraction eg. 0.6666666) ");
  params.addParam<Real>("kb",
                        1.0,
                        "Site fraction for the cb variable (specify this if ca is a sublattice "
                        "concentration, and make sure it is a true site fraction eg. 0.6666666) ");
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
    _cb_name(coupledName("cb", 0)),
    // first derivatives
    _dfadca(getMaterialPropertyDerivative<Real>("fa_name", _var.name())),
    _dfbdcb(getMaterialPropertyDerivative<Real>("fb_name", _cb_name)),
    // second derivatives d2F/dx*dca for jacobian diagonal elements
    _d2fadca2(getMaterialPropertyDerivative<Real>("fa_name", _var.name(), _var.name())),
    _d2fbdcbca(getMaterialPropertyDerivative<Real>("fb_name", _cb_name, _var.name())),
    _d2fadcadarg(_n_args),
    _d2fbdcbdarg(_n_args),
    // site fractions
    _ka(getParam<Real>("ka")),
    _kb(getParam<Real>("kb"))
{
#ifdef DEBUG
  _console << "KKSPhaseChemicalPotential(" << name() << ") " << _var.name() << ' ' << _cb_name
           << '\n';
#endif

  // lookup table for the material properties representing the derivatives needed for the
  // off-diagonal jacobian
  for (std::size_t i = 0; i < _n_args; ++i)
  {
    _d2fadcadarg[i] = &getMaterialPropertyDerivative<Real>("fa_name", _var.name(), i);
    _d2fbdcbdarg[i] = &getMaterialPropertyDerivative<Real>("fb_name", _cb_name, i);
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
  return _test[_i][_qp] * (_dfadca[_qp] / _ka - _dfbdcb[_qp] / _kb);
}

Real
KKSPhaseChemicalPotential::computeQpJacobian()
{
  // for on diagonal we return the d/dca derivative of the residual
  return _test[_i][_qp] * _phi[_j][_qp] * (_d2fadca2[_qp] / _ka - _d2fbdcbca[_qp] / _kb);
}

Real
KKSPhaseChemicalPotential::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return _test[_i][_qp] * _phi[_j][_qp] *
         ((*_d2fadcadarg[cvar])[_qp] / _ka - (*_d2fbdcbdarg[cvar])[_qp] / _kb);
}
