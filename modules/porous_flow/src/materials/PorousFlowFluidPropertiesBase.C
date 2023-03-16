//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidPropertiesBase.h"

template <bool is_ad>
InputParameters
PorousFlowFluidPropertiesBaseTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterialBase::validParams();
  MooseEnum unit_choice("Kelvin=0 Celsius=1", "Kelvin");
  params.addParam<MooseEnum>(
      "temperature_unit", unit_choice, "The unit of the temperature variable");
  params.addPrivateParam<std::string>("pf_material_type", "fluid_properties");
  params.addPrivateParam<bool>("is_ad", is_ad);
  params.addClassDescription("Base class for PorousFlow fluid materials");
  return params;
}

template <bool is_ad>
PorousFlowFluidPropertiesBaseTempl<is_ad>::PorousFlowFluidPropertiesBaseTempl(
    const InputParameters & parameters)
  : PorousFlowMaterialBase(parameters),
    _porepressure(
        _nodal_material
            ? getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_porepressure_nodal")
            : getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_porepressure_qp")),
    _temperature(_nodal_material
                     ? getGenericMaterialProperty<Real, is_ad>("PorousFlow_temperature_nodal")
                     : getGenericMaterialProperty<Real, is_ad>("PorousFlow_temperature_qp")),
    _t_c2k(getParam<MooseEnum>("temperature_unit") == 0 ? 0.0 : 273.15),
    _R(8.3144598)
{
}

template <bool is_ad>
void
PorousFlowFluidPropertiesBaseTempl<is_ad>::computeQpProperties()
{
  mooseError("computeQpProperties() must be overriden in materials derived from "
             "PorousFlowFluidPropertiesBase");
}

template class PorousFlowFluidPropertiesBaseTempl<false>;
template class PorousFlowFluidPropertiesBaseTempl<true>;
