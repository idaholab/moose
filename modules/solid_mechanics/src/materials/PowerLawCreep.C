/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PowerLawCreep.h"

template <>
InputParameters
validParams<PowerLawCreep>()
{
  InputParameters params = validParams<SolidModel>();

  //   Power-law creep material parameters
  params.addRequiredParam<Real>("coefficient", "Leading coefficent in power-law equation");
  params.addRequiredParam<Real>("n_exponent", "Exponent on effective stress in power-law equation");
  params.addParam<Real>("m_exponent", 0.0, "Exponent on time in power-law equation");
  params.addRequiredParam<Real>("activation_energy", "Activation energy");
  params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
  params.addParam<Real>("start_time", 0, "Start time (if not zero)");

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

PowerLawCreep::PowerLawCreep(const InputParameters & parameters) : SolidModel(parameters)
{

  createConstitutiveModel("PowerLawCreepModel");
}
