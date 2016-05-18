/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFluidPropertiesBase.h"
#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowFluidPropertiesBase>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<unsigned int>("phase", "The phase number");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("Base class for PorousFlow fluid materials");
  return params;
}

PorousFlowFluidPropertiesBase::PorousFlowFluidPropertiesBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _phase_num(getParam<unsigned int>("phase")),
    _porepressure_nodal(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_nodal")),
    _porepressure_qp(getMaterialProperty<std::vector<Real> >("PorousFlow_porepressure_qp")),
    _temperature_nodal(getMaterialProperty<std::vector<Real> >("PorousFlow_temperature_nodal")),
    _temperature_qp(getMaterialProperty<std::vector<Real> >("PorousFlow_temperature_qp")),
    _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
    _pressure_variable_name(_dictator_UO.pressureVariableNameDummy()),
    _temperature_variable_name(_dictator_UO.temperatureVariableNameDummy()),
    _t_c2k(273.15),
    _R(8.3144621)
{
  if (_phase_num >= _dictator_UO.numPhases())
    mooseError("PorousFlowFluidProperties: The Dictator proclaims that the number of fluid phases is " << _dictator_UO.numPhases() << " while you have foolishly entered phase = " << _phase_num << " in " << _name << ".  Be aware that the Dictator does not tolerate mistakes.");
}

void
PorousFlowFluidPropertiesBase::computeQpProperties()
{
  mooseError("computeQpProperties() must be overriden in materials derived from PorousFlowFluidPropertiesBase");
}

