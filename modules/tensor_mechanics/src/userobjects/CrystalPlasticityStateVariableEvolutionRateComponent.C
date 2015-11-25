/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrystalPlasticityStateVariableEvolutionRateComponent.h"

template<>
InputParameters validParams<CrystalPlasticityStateVariableEvolutionRateComponent>()
{
  InputParameters params = validParams<CrystalPlasticityUOBase>();
  params.addClassDescription("Crystal plasticity state variable evolution rate component base class.  Override the virtual functions in your class");
  return params;
}

CrystalPlasticityStateVariableEvolutionRateComponent::CrystalPlasticityStateVariableEvolutionRateComponent(const InputParameters & parameters) :
    CrystalPlasticityUOBase(parameters)
{
}
