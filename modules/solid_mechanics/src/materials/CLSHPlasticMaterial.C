/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
