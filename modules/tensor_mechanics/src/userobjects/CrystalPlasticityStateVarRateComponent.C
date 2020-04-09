//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticityStateVarRateComponent.h"

InputParameters
CrystalPlasticityStateVarRateComponent::validParams()
{
  InputParameters params = CrystalPlasticityUOBase::validParams();
  params.addClassDescription("Crystal plasticity state variable evolution rate component base "
                             "class.  Override the virtual functions in your class");
  return params;
}

CrystalPlasticityStateVarRateComponent::CrystalPlasticityStateVarRateComponent(
    const InputParameters & parameters)
  : CrystalPlasticityUOBase(parameters)
{
}
