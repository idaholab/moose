//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityBase.h"

template <bool is_ad>
InputParameters
PorousFlowPorosityBaseTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addPrivateParam<std::string>("pf_material_type", "porosity");
  params.addRangeCheckedParam<Real>(
      "porosity_min",
      "porosity_min >= 0",
      "Minimum allowed value of the porosity: if the computed porosity is less than this value, "
      "porosity is set to this value instead.  If not provided, no lower bound is imposed");
  params.addRangeCheckedParam<Real>(
      "zero_modifier",
      1E-3,
      "zero_modifier >= 0",
      "If the porosity_min floor is active, the porosity derivatives are set to zero_modifier "
      "times their unfloored values (rather than exactly zero) to hint to the nonlinear solver "
      "that porosity is not strictly constant, which aids convergence");
  params.addParamNamesToGroup("zero_modifier", "Advanced");
  params.addClassDescription("Base class Material for porosity");
  return params;
}

template <bool is_ad>
PorousFlowPorosityBaseTempl<is_ad>::PorousFlowPorosityBaseTempl(const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _porosity(_nodal_material ? declareGenericProperty<Real, is_ad>("PorousFlow_porosity_nodal")
                              : declareGenericProperty<Real, is_ad>("PorousFlow_porosity_qp")),
    _dporosity_dvar(is_ad ? nullptr
                    : _nodal_material
                        ? &declareProperty<std::vector<Real>>("dPorousFlow_porosity_nodal_dvar")
                        : &declareProperty<std::vector<Real>>("dPorousFlow_porosity_qp_dvar")),
    _dporosity_dgradvar(
        is_ad ? nullptr
        : _nodal_material
            ? &declareProperty<std::vector<RealGradient>>("dPorousFlow_porosity_nodal_dgradvar")
            : &declareProperty<std::vector<RealGradient>>("dPorousFlow_porosity_qp_dgradvar")),
    _porosity_min(isParamValid("porosity_min") ? getParam<Real>("porosity_min")
                                               : std::numeric_limits<Real>::lowest()),
    _zero_modifier(getParam<Real>("zero_modifier"))
{
}

template <bool is_ad>
void
PorousFlowPorosityBaseTempl<is_ad>::applyPorosityMin()
{
  if (MetaPhysicL::raw_value(_porosity[_qp]) >= _porosity_min)
    return;

  if constexpr (!is_ad)
  {
    _porosity[_qp] = _porosity_min;
    for (unsigned int v = 0; v < _num_var; ++v)
    {
      (*_dporosity_dvar)[_qp][v] *= _zero_modifier;
      (*_dporosity_dgradvar)[_qp][v] *= _zero_modifier;
    }
  }
  else
  {
    // The AD path carries its derivatives inside the value and has no derivative material
    // properties, so the value and the derivatives are softened separately.  Assigning
    // _porosity_min to the ADReal would instead discard the derivatives entirely.
    _porosity[_qp].value() = _porosity_min;
    _porosity[_qp].derivatives() *= _zero_modifier;
  }
}

template class PorousFlowPorosityBaseTempl<false>;
template class PorousFlowPorosityBaseTempl<true>;
