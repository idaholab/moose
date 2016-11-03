/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFluidPropertiesBase.h"

template<>
InputParameters validParams<PorousFlowFluidPropertiesBase>()
{
  InputParameters params = validParams<PorousFlowMaterialBase>();
  params.addClassDescription("Base class for PorousFlow fluid materials");
  return params;
}

PorousFlowFluidPropertiesBase::PorousFlowFluidPropertiesBase(const InputParameters & parameters) :
    PorousFlowMaterialBase(parameters),
    _porepressure_nodal(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_nodal")),
    _porepressure_qp(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_qp")),
    _temperature_nodal(getMaterialProperty<Real>("PorousFlow_temperature_nodal")),
    _temperature_qp(getMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _pressure_variable_name(_dictator.pressureVariableNameDummy()),
    _temperature_variable_name(_dictator.temperatureVariableNameDummy()),
    _t_c2k(273.15),
    _R(8.3144621)
{
}

void
PorousFlowFluidPropertiesBase::computeQpProperties()
{
  mooseError("computeQpProperties() must be overriden in materials derived from PorousFlowFluidPropertiesBase");
}
