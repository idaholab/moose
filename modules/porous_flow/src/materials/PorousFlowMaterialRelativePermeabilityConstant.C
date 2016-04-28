/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterialRelativePermeabilityConstant.h"

template<>
InputParameters validParams<PorousFlowMaterialRelativePermeabilityConstant>()
{
  InputParameters params = validParams<PorousFlowMaterialRelativePermeabilityBase>();
  params.addClassDescription("This Material provides a constant relative permeability (1.0)");
  return params;
}

PorousFlowMaterialRelativePermeabilityConstant::PorousFlowMaterialRelativePermeabilityConstant(const InputParameters & parameters) :
    PorousFlowMaterialRelativePermeabilityBase(parameters)
{
}

void
PorousFlowMaterialRelativePermeabilityConstant::computeQpProperties()
{
  _relative_permeability[_qp] = 1.0;
  _drelative_permeability_ds[_qp] = 0.0;
}
