//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSPhaseChemicalPotential.h"

registerMooseObject("PhaseFieldApp", KKSPhaseChemicalPotential);
registerMooseObject("PhaseFieldApp", ADKKSPhaseChemicalPotential);

template <bool is_ad>
InputParameters
KKSPhaseChemicalPotentialTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription("KKS model kernel to enforce the pointwise equality of phase chemical "
                             "potentials $dF_a/dc_a = dF_b/dc_b$. The nonlinear variable of this "
                             "kernel is $c_a$.");
  params.addRequiredCoupledVar(
      "cb", "Phase b concentration"); // ca is the nonlinear variable of this kernel
  params.addRequiredParam<MaterialPropertyName>(
      "fa_name",
      "Base name of the phase-a free-energy function (the property_name/f_name of its derivative "
      "material)");
  params.addRequiredParam<MaterialPropertyName>(
      "fb_name",
      "Base name of the phase-b free-energy function (the property_name/f_name of its derivative "
      "material)");
  params.addParam<Real>(
      "ka",
      1.0,
      "Site fraction for ca when ca is a sublattice concentration; use 1 for ordinary KKS");
  params.addParam<Real>(
      "kb",
      1.0,
      "Site fraction for cb when cb is a sublattice concentration; use 1 for ordinary KKS");
  params.addCoupledVar(
      "args_a",
      "Additional nonlinear variables on which Fa depends; needed by the non-AD Jacobian and "
      "retained for equivalent AD/non-AD coupling declarations");
  params.addCoupledVar(
      "args_b",
      "Additional nonlinear variables on which Fb depends; needed by the non-AD Jacobian and "
      "retained for equivalent AD/non-AD coupling declarations");
  return params;
}

template <bool is_ad>
KKSPhaseChemicalPotentialTempl<is_ad>::KKSPhaseChemicalPotentialTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<GenericKernel<is_ad>>>(parameters),
    _cb_name(this->coupledName("cb")),
    _fa_name(this->template getParam<MaterialPropertyName>("fa_name")),
    _fb_name(this->template getParam<MaterialPropertyName>("fb_name")),
    _dfadca(this->template getMaterialPropertyDerivativeByName<Real, is_ad>(_fa_name, _var.name())),
    _dfbdcb(this->template getMaterialPropertyDerivativeByName<Real, is_ad>(_fb_name, _cb_name)),
    _ka(this->template getParam<Real>("ka")),
    _kb(this->template getParam<Real>("kb"))
{
#ifdef DEBUG
  _console << "KKSPhaseChemicalPotential(" << this->name() << ") " << _var.name() << ' ' << _cb_name
           << '\n';
#endif
}

template <bool is_ad>
void
KKSPhaseChemicalPotentialTempl<is_ad>::initialSetup()
{
  // U is Real and is_ad selects MaterialProperty<Real> or ADMaterialProperty<Real>.
  this->template validateNonlinearCoupling<Real, is_ad>(_fa_name);
  this->template validateNonlinearCoupling<Real, is_ad>(_fb_name);
}

template <bool is_ad>
GenericReal<is_ad>
KKSPhaseChemicalPotentialTempl<is_ad>::computeQpResidual()
{
  return _test[_i][_qp] * (_dfadca[_qp] / _ka - _dfbdcb[_qp] / _kb);
}

KKSPhaseChemicalPotential::KKSPhaseChemicalPotential(const InputParameters & parameters)
  : KKSPhaseChemicalPotentialTempl<false>(parameters),
    _d2fadca2(getMaterialPropertyDerivativeByName<Real>(_fa_name, _var.name(), _var.name())),
    _d2fbdcbca(getMaterialPropertyDerivativeByName<Real>(_fb_name, _cb_name, _var.name())),
    _d2fadcadarg(_n_args),
    _d2fbdcbdarg(_n_args)
{
  for (std::size_t i = 0; i < _n_args; ++i)
  {
    const VariableName & arg_name = _coupled_standard_moose_vars[i]->name();
    _d2fadcadarg[i] = &getMaterialPropertyDerivativeByName<Real>(_fa_name, _var.name(), arg_name);
    _d2fbdcbdarg[i] = &getMaterialPropertyDerivativeByName<Real>(_fb_name, _cb_name, arg_name);
  }
}

Real
KKSPhaseChemicalPotential::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp] * (_d2fadca2[_qp] / _ka - _d2fbdcbca[_qp] / _kb);
}

Real
KKSPhaseChemicalPotential::computeQpOffDiagJacobian(unsigned int jvar)
{
  const unsigned int cvar = mapJvarToCvar(jvar);

  return _test[_i][_qp] * _phi[_j][_qp] *
         ((*_d2fadcadarg[cvar])[_qp] / _ka - (*_d2fbdcbdarg[cvar])[_qp] / _kb);
}

template class KKSPhaseChemicalPotentialTempl<false>;
template class KKSPhaseChemicalPotentialTempl<true>;
