/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LinearStrainHardening.h"

template <>
InputParameters
validParams<LinearStrainHardening>()
{
  InputParameters params = validParams<SolidModel>();

  //  Linear strain hardening parameters
  params.addRequiredParam<Real>("yield_stress",
                                "The point at which plastic strain begins accumulating");
  params.addRequiredParam<Real>("hardening_constant", "Hardening slope");
  params.addParam<FunctionName>("hardening_function",
                                "True stress as a function of plastic strain");

  //  Sub-Newton Iteration control parameters
  params.addParam<Real>(
      "relative_tolerance", 1e-5, "Relative convergence tolerance for sub-newtion iteration");
  params.addParam<Real>(
      "absolute_tolerance", 1e-20, "Absolute convergence tolerance for sub-newtion iteration");
  params.addParam<unsigned int>("max_its", 10, "Maximum number of sub-newton iterations");
  params.addParam<bool>(
      "output_iteration_info", false, "Set true to output sub-newton iteration information");
  params.addParam<bool>("output_iteration_info_on_error",
                        false,
                        "Set true to output sub-newton iteration information when a step fails");

  return params;
}

LinearStrainHardening::LinearStrainHardening(const InputParameters & parameters)
  : SolidModel(parameters)
{

  createConstitutiveModel("IsotropicPlasticity");
}
