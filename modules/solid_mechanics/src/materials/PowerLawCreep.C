/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PowerLawCreep.h"
#include "PowerLawCreepModel.h"

template <>
InputParameters
validParams<PowerLawCreep>()
{
  InputParameters params = validParams<SolidModel>();
  params += validParams<PowerLawCreepModel>();

  return params;
}

PowerLawCreep::PowerLawCreep(const InputParameters & parameters) : SolidModel(parameters)
{

  createConstitutiveModel("PowerLawCreepModel");
}
