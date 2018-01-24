/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LinearStrainHardening.h"
#include "IsotropicPlasticity.h"

template <>
InputParameters
validParams<LinearStrainHardening>()
{
  InputParameters params = validParams<SolidModel>();
  params += validParams<IsotropicPlasticity>();

  return params;
}

LinearStrainHardening::LinearStrainHardening(const InputParameters & parameters)
  : SolidModel(parameters)
{

  createConstitutiveModel("IsotropicPlasticity");
}
