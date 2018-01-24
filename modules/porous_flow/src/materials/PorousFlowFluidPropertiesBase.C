//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidPropertiesBase.h"

template <>
InputParameters
validParams<PorousFlowFluidPropertiesBase>()
{
  InputParameters params = validParams<PorousFlowMaterialBase>();
  MooseEnum unit_choice("Kelvin=0 Celsius=1", "Kelvin");
  params.addParam<MooseEnum>(
      "temperature_unit", unit_choice, "The unit of the temperature variable");
  params.addClassDescription("Base class for PorousFlow fluid materials");
  return params;
}

PorousFlowFluidPropertiesBase::PorousFlowFluidPropertiesBase(const InputParameters & parameters)
  : PorousFlowMaterialBase(parameters),
    _porepressure(_nodal_material
                      ? getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_nodal")
                      : getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _temperature(_nodal_material ? getMaterialProperty<Real>("PorousFlow_temperature_nodal")
                                 : getMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _pressure_variable_name(_dictator.pressureVariableNameDummy()),
    _temperature_variable_name(_dictator.temperatureVariableNameDummy()),
    _t_c2k(getParam<MooseEnum>("temperature_unit") == 0 ? 0.0 : 273.15),
    _R(8.3144598)
{
}

void
PorousFlowFluidPropertiesBase::computeQpProperties()
{
  mooseError("computeQpProperties() must be overriden in materials derived from "
             "PorousFlowFluidPropertiesBase");
}
