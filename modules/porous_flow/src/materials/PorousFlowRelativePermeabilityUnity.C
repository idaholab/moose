/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowRelativePermeabilityUnity.h"

template<>
InputParameters validParams<PorousFlowRelativePermeabilityUnity>()
{
  InputParameters params = validParams<PorousFlowRelativePermeabilityBase>();
  params.addClassDescription("This class sets the relative permeability = 1");
  return params;
}

PorousFlowRelativePermeabilityUnity::PorousFlowRelativePermeabilityUnity(const InputParameters & parameters) :
    PorousFlowRelativePermeabilityBase(parameters)
{
}

void
PorousFlowRelativePermeabilityUnity::computeQpProperties()
{
  _relative_permeability[_qp] = 1.0;
  _drelative_permeability_ds[_qp] = 0.0;
}
