//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CLSHPlasticMaterial.h"
#include "CLSHPlasticModel.h"

template <>
InputParameters
validParams<CLSHPlasticMaterial>()
{
  InputParameters params = validParams<SolidModel>();
  params += validParams<CLSHPlasticModel>();
  return params;
}

CLSHPlasticMaterial::CLSHPlasticMaterial(const InputParameters & parameters)
  : SolidModel(parameters)
{

  createConstitutiveModel("CLSHPlasticModel");
}
