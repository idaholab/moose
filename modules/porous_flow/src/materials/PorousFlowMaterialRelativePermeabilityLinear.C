/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialRelativePermeabilityLinear.h"

template<>
InputParameters validParams<PorousFlowMaterialRelativePermeabilityLinear>()
{
  InputParameters params = validParams<PorousFlowMaterialRelativePermeabilityBase>();
  params.addClassDescription("This Material provides a linear relative permeability");
  return params;
}

PorousFlowMaterialRelativePermeabilityLinear::PorousFlowMaterialRelativePermeabilityLinear(const InputParameters & parameters) :
  PorousFlowMaterialRelativePermeabilityBase(parameters)
{
}

void
PorousFlowMaterialRelativePermeabilityLinear::computeQpProperties()
{
  /// The relative permeability is equal to the phase saturation and its derivative
  /// wrt saturation is 1
  _relative_permeability[_qp] = _saturation[_qp][_phase_num];
  _drelative_permeability_ds[_qp] = 1.0;
}
