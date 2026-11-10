//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDesorpedMassVolumetricExpansion.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowDesorpedMassVolumetricExpansion);
registerMooseObject("PorousFlowApp", ADPorousFlowDesorpedMassVolumetricExpansion);

template <bool is_ad>
InputParameters
PorousFlowDesorpedMassVolumetricExpansionTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addRequiredCoupledVar(
      "conc_var", "The variable that represents the concentration of desorped species");
  params.addClassDescription("Desorped_mass * rate_of_solid_volumetric_expansion");
  return params;
}

template <bool is_ad>
PorousFlowDesorpedMassVolumetricExpansionTempl<
    is_ad>::PorousFlowDesorpedMassVolumetricExpansionTempl(const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _dictator(this->template getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _conc_var_number(coupled("conc_var")),
    _conc(this->template coupledGenericValue<is_ad>("conc_var")),
    _porosity(this->template getGenericMaterialProperty<Real, is_ad>("PorousFlow_porosity_qp")),
    _dporosity_dvar(is_ad ? nullptr
                          : &this->template getMaterialProperty<std::vector<Real>>(
                                "dPorousFlow_porosity_qp_dvar")),
    _dporosity_dgradvar(is_ad ? nullptr
                              : &this->template getMaterialProperty<std::vector<RealGradient>>(
                                    "dPorousFlow_porosity_qp_dgradvar")),
    _strain_rate_qp(this->template getGenericMaterialProperty<Real, is_ad>(
        "PorousFlow_volumetric_strain_rate_qp")),
    _dstrain_rate_qp_dvar(is_ad ? nullptr
                                : &this->template getMaterialProperty<std::vector<RealGradient>>(
                                      "dPorousFlow_volumetric_strain_rate_qp_dvar"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowDesorpedMassVolumetricExpansionTempl<is_ad>::computeQpResidual()
{
  return _test[_i][_qp] * (1.0 - _porosity[_qp]) * _conc[_qp] * _strain_rate_qp[_qp];
}

template <bool is_ad>
Real
PorousFlowDesorpedMassVolumetricExpansionTempl<is_ad>::computeQpJacobian()
{
  return computeQpJac(_var.number());
}

template <bool is_ad>
Real
PorousFlowDesorpedMassVolumetricExpansionTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  return computeQpJac(jvar);
}

template <bool is_ad>
Real
PorousFlowDesorpedMassVolumetricExpansionTempl<is_ad>::computeQpJac(unsigned int jvar) const
{
  if constexpr (!is_ad)
  {
    Real deriv = 0.0;

    if (jvar == _conc_var_number)
      deriv = (1.0 - _porosity[_qp]) * _phi[_j][_qp] * _strain_rate_qp[_qp];

    if (_dictator.notPorousFlowVariable(jvar))
      return _test[_i][_qp] * deriv;
    const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);

    deriv -=
        (*_dporosity_dgradvar)[_qp][pvar] * _grad_phi[_j][_qp] * _conc[_qp] * _strain_rate_qp[_qp];
    deriv -= (*_dporosity_dvar)[_qp][pvar] * _phi[_j][_qp] * _conc[_qp] * _strain_rate_qp[_qp];
    deriv += (1.0 - _porosity[_qp]) * _conc[_qp] * (*_dstrain_rate_qp_dvar)[_qp][pvar] *
             _grad_phi[_j][_qp];

    return _test[_i][_qp] * deriv;
  }
  else
  {
    libmesh_ignore(jvar);
    return 0.0;
  }
}

template class PorousFlowDesorpedMassVolumetricExpansionTempl<false>;
template class PorousFlowDesorpedMassVolumetricExpansionTempl<true>;
