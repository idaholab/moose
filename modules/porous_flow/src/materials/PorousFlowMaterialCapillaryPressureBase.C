/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialCapillaryPressureBase.h"
#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialCapillaryPressureBase>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<unsigned int>("phase", "The phase number");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("Base class for PorousFlow capillary pressure materials");
  return params;
}

PorousFlowMaterialCapillaryPressureBase::PorousFlowMaterialCapillaryPressureBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

  _phase_num(getParam<unsigned int>("phase")),
  _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
  _saturation_variable_name(_dictator_UO.saturationVariableNameDummy()),
  _saturation(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation")),
  _capillary_pressure(declareProperty<Real>("PorousFlow_capillary_pressure" + Moose::stringify(_phase_num))),
  _dcapillary_pressure_ds(declarePropertyDerivative<Real>("PorousFlow_capillary_pressure" + Moose::stringify(_phase_num), _saturation_variable_name))
{
  if (_phase_num >= _dictator_UO.num_phases())
    mooseError("PorousFlowMaterialCapillaryPressure: The Dictator proclaims that the number of fluid phases is " << _dictator_UO.num_phases() << " while you have foolishly entered phase = " << _phase_num << ".  Be aware that the Dictator does not tolerate mistakes.");
}

void
PorousFlowMaterialCapillaryPressureBase::computeQpProperties()
{
  mooseError("computeQpProperties() must be overriden in materials derived from PorousFlowMaterialCapillaryPressureBase");
}
