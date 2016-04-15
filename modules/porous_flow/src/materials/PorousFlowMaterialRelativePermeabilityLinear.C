/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialRelativePermeabilityLinear.h"
#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialRelativePermeabilityLinear>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addRequiredParam<unsigned int>("phase", "The phase number");
  params.addClassDescription("This Material provides a linear relative permeability");
  return params;
}

PorousFlowMaterialRelativePermeabilityLinear::PorousFlowMaterialRelativePermeabilityLinear(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

  _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
  _phase_num(getParam<unsigned int>("phase")),
  _saturation_variable_name("saturation_varname"),
  _relative_permeability(declareProperty<Real>("PorousFlow_relative_permeability" + Moose::stringify(_phase_num))),
  _drelative_permeability_ds(declarePropertyDerivative<Real>("PorousFlow_relative_permeability" + Moose::stringify(_phase_num), _saturation_variable_name)),
  _saturation(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation"))
{
}

void
PorousFlowMaterialRelativePermeabilityLinear::computeQpProperties()
{
  /// The relative permeability is equal to the phase saturation
  _relative_permeability[_qp] = _saturation[_qp][_phase_num];
  _drelative_permeability_ds[_qp] = 1.0;
}
