/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowRelativePermeabilityBase.h"

template<>
InputParameters validParams<PorousFlowRelativePermeabilityBase>()
{
  InputParameters params = validParams<PorousFlowMaterialBase>();
  params.addClassDescription("Base class for PorousFlow relative permeability materials");
  return params;
}

PorousFlowRelativePermeabilityBase::PorousFlowRelativePermeabilityBase(const InputParameters & parameters) :
    PorousFlowMaterialBase(parameters),
    _saturation_variable_name(_dictator.saturationVariableNameDummy()),
    _saturation_nodal(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_nodal")),
    _relative_permeability(declareProperty<Real>("PorousFlow_relative_permeability" + _phase)),
    _drelative_permeability_ds(declarePropertyDerivative<Real>("PorousFlow_relative_permeability" + _phase, _saturation_variable_name))
{
}

void
PorousFlowRelativePermeabilityBase::computeQpProperties()
{
  mooseError("computeQpProperties() must be overriden in materials derived from PorousFlowRelativePermeabilityBase");
}
