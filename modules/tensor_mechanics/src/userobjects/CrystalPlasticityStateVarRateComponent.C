/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrystalPlasticityStateVarRateComponent.h"

template <>
InputParameters
validParams<CrystalPlasticityStateVarRateComponent>()
{
  InputParameters params = validParams<CrystalPlasticityUOBase>();
  params.addClassDescription("Crystal plasticity state variable evolution rate component base "
                             "class.  Override the virtual functions in your class");
  return params;
}

CrystalPlasticityStateVarRateComponent::CrystalPlasticityStateVarRateComponent(
    const InputParameters & parameters)
  : CrystalPlasticityUOBase(parameters)
{
}
