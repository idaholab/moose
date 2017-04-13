/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CLSHPlasticMaterial.h"

template <>
InputParameters
validParams<CLSHPlasticMaterial>()
{
  InputParameters params = validParams<SolidModel>();
  params.addRequiredParam<Real>("yield_stress",
                                "The point at which plastic strain begins accumulating");
  params.addRequiredParam<Real>("hardening_constant", "Hardening slope");
  params.addRequiredParam<Real>("c_alpha", "creep constant");
  params.addRequiredParam<Real>("c_beta", "creep constant");
  params.addParam<unsigned int>("max_its", 30, "Maximum number of sub-newton iterations");
  params.addParam<bool>(
      "output_iteration_info", false, "Set true to output sub-newton iteration information");
  params.addParam<bool>("output_iteration_info_on_error",
                        false,
                        "Set true to output sub-newton iteration information when a step fails");
  params.addParam<Real>(
      "relative_tolerance", 1e-5, "Relative convergence tolerance for sub-newtion iteration");
  params.addParam<Real>(
      "absolute_tolerance", 1e-20, "Absolute convergence tolerance for sub-newtion iteration");
  return params;
}

CLSHPlasticMaterial::CLSHPlasticMaterial(const InputParameters & parameters)
  : SolidModel(parameters)
{

  createConstitutiveModel("CLSHPlasticModel");
}
