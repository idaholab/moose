//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
