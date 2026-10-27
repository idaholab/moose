//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowEffectiveStressCoupling.h"

#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowEffectiveStressCoupling);
registerMooseObject("PorousFlowApp", ADPorousFlowEffectiveStressCoupling);

template <bool is_ad>
InputParameters
PorousFlowEffectiveStressCouplingTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription("Implements the weak form of the expression biot_coefficient * "
                             "grad(effective fluid pressure)");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addRangeCheckedParam<Real>(
      "biot_coefficient", 1, "biot_coefficient>=0&biot_coefficient<=1", "Biot coefficient");
  params.addRequiredParam<unsigned int>("component",
                                        "The component (0 for x, 1 for y and 2 for z) of grad(P)");
  return params;
}

template <bool is_ad>
PorousFlowEffectiveStressCouplingTempl<is_ad>::PorousFlowEffectiveStressCouplingTempl(
    const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _dictator(this->template getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _coefficient(this->template getParam<Real>("biot_coefficient")),
    _component(this->template getParam<unsigned int>("component")),
    _pf(this->template getGenericMaterialProperty<Real, is_ad>(
        "PorousFlow_effective_fluid_pressure_qp")),
    _dpf_dvar(is_ad ? nullptr
                    : &this->template getMaterialProperty<std::vector<Real>>(
                          "dPorousFlow_effective_fluid_pressure_qp_dvar")),
    _rz(this->getBlockCoordSystem() == Moose::COORD_RZ)
{
  if (_component >= this->_mesh.dimension())
    this->paramError("component", "The component cannot be greater than the mesh dimension");
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowEffectiveStressCouplingTempl<is_ad>::computeQpResidual()
{
  if (_rz && _component == 0)
    return -_coefficient * _pf[_qp] * (_grad_test[_i][_qp](0) + _test[_i][_qp] / _q_point[_qp](0));
  return -_coefficient * _pf[_qp] * _grad_test[_i][_qp](_component);
}

template <bool is_ad>
Real
PorousFlowEffectiveStressCouplingTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    if (_dictator.notPorousFlowVariable(_var.number()))
      return 0.0;
    const unsigned int pvar = _dictator.porousFlowVariableNum(_var.number());
    if (_rz && _component == 0)
      return -_coefficient * _phi[_j][_qp] * (*_dpf_dvar)[_qp][pvar] *
             (_grad_test[_i][_qp](0) + _test[_i][_qp] / _q_point[_qp](0));
    return -_coefficient * _phi[_j][_qp] * (*_dpf_dvar)[_qp][pvar] *
           _grad_test[_i][_qp](_component);
  }
  return 0.0;
}

template <bool is_ad>
Real
PorousFlowEffectiveStressCouplingTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  if constexpr (!is_ad)
  {
    if (_dictator.notPorousFlowVariable(jvar))
      return 0.0;
    const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);
    if (_rz && _component == 0)
      return -_coefficient * _phi[_j][_qp] * (*_dpf_dvar)[_qp][pvar] *
             (_grad_test[_i][_qp](0) + _test[_i][_qp] / _q_point[_qp](0));
    return -_coefficient * _phi[_j][_qp] * (*_dpf_dvar)[_qp][pvar] *
           _grad_test[_i][_qp](_component);
  }
  else
    libmesh_ignore(jvar);
  return 0.0;
}

template class PorousFlowEffectiveStressCouplingTempl<false>;
template class PorousFlowEffectiveStressCouplingTempl<true>;
