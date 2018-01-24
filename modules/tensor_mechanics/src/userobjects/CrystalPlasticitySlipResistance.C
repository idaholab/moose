/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrystalPlasticitySlipResistance.h"

template <>
InputParameters
validParams<CrystalPlasticitySlipResistance>()
{
  InputParameters params = validParams<CrystalPlasticityUOBase>();
  params.addClassDescription("Crystal plasticity slip resistance base class.  Override the virtual "
                             "functions in your class");
  return params;
}

CrystalPlasticitySlipResistance::CrystalPlasticitySlipResistance(const InputParameters & parameters)
  : CrystalPlasticityUOBase(parameters)
{
}
