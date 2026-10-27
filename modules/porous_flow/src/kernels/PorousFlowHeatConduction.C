//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHeatConduction.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowHeatConduction);
registerMooseObject("PorousFlowApp", ADPorousFlowHeatConduction);

template <bool is_ad>
InputParameters
PorousFlowHeatConductionTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("Heat conduction in the Porous Flow module");
  return params;
}

template <bool is_ad>
PorousFlowHeatConductionTempl<is_ad>::PorousFlowHeatConductionTempl(
    const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _dictator(this->template getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _la(this->template getGenericMaterialProperty<RealTensorValue, is_ad>(
        "PorousFlow_thermal_conductivity_qp")),
    _dla_dvar(is_ad ? nullptr
                    : &this->template getMaterialProperty<std::vector<RealTensorValue>>(
                          "dPorousFlow_thermal_conductivity_qp_dvar")),
    _grad_t(this->template getGenericMaterialProperty<RealGradient, is_ad>(
        "PorousFlow_grad_temperature_qp")),
    _dgrad_t_dvar(is_ad ? nullptr
                        : &this->template getMaterialProperty<std::vector<RealGradient>>(
                              "dPorousFlow_grad_temperature_qp_dvar")),
    _dgrad_t_dgradvar(is_ad ? nullptr
                            : &this->template getMaterialProperty<std::vector<Real>>(
                                  "dPorousFlow_grad_temperature_qp_dgradvar"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowHeatConductionTempl<is_ad>::computeQpResidual()
{
  return _grad_test[_i][_qp] * (_la[_qp] * _grad_t[_qp]);
}

template <bool is_ad>
Real
PorousFlowHeatConductionTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
    return computeQpOffDiagJacobian(_var.number());
  return 0.0;
}

template <bool is_ad>
Real
PorousFlowHeatConductionTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  if constexpr (!is_ad)
  {
    if (_dictator.notPorousFlowVariable(jvar))
      return 0.0;
    const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);
    return _grad_test[_i][_qp] *
           (((*_dla_dvar)[_qp][pvar] * _grad_t[_qp] + _la[_qp] * (*_dgrad_t_dvar)[_qp][pvar]) *
                _phi[_j][_qp] +
            _la[_qp] * (*_dgrad_t_dgradvar)[_qp][pvar] * _grad_phi[_j][_qp]);
  }
  else
    libmesh_ignore(jvar);
  return 0.0;
}

template class PorousFlowHeatConductionTempl<false>;
template class PorousFlowHeatConductionTempl<true>;
