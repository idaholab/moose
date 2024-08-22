//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonsafeMaterial.h"
#include "MooseApp.h"

registerMooseObject("MooseTestApp", NonsafeMaterial);

InputParameters
NonsafeMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<Real>("diffusivity", "The diffusivity value.");
  params.addParam<Real>(
      "threshold",
      0,
      "This value sets the upper limit for the validity of the material property value.");
  params.addParam<bool>("test_different_procs",
                        false,
                        "True to test setting invalid solutions on different processors");
  params.addParam<bool>("test_invalid_recover", false, "True to test invalid solutions recover");
  params.addParam<Real>("invalid_after_time",
                        -std::numeric_limits<Real>::max(),
                        "Only set invalid solutions after this time");
  params.addParam<bool>(
      "flag_solution_warning",
      false,
      "True to test an invalid solution warning, which do not cause the solution to be invalid.");

  return params;
}

NonsafeMaterial::NonsafeMaterial(const InputParameters & parameters)
  : Material(parameters),
    _input_diffusivity(getParam<Real>("diffusivity")),
    _threshold(getParam<Real>("threshold")),
    _diffusivity(declareProperty<Real>("diffusivity")),
    _test_different_procs(getParam<bool>("test_different_procs")),
    _test_invalid_recover(getParam<bool>("test_invalid_recover")),
    _invalid_after_time(getParam<Real>("invalid_after_time")),
    _flag_solution_warning(getParam<bool>("flag_solution_warning"))
{
}

void
NonsafeMaterial::computeQpProperties()
{
  Real _test_diffusivity = _input_diffusivity;
  // Gradually modify diffusivity value to test solution invalid recover
  if (_fe_problem.dt() < 1 && _test_invalid_recover)
    _test_diffusivity /= 2;
  if (_test_diffusivity > _threshold && _fe_problem.time() > _invalid_after_time)
  {
    if (_flag_solution_warning)
      flagSolutionWarning("Solution invalid warning!");
    else
    {
      if (!_test_different_procs || processor_id() == 0)
        flagInvalidSolution("The diffusivity is greater than the threshold value!");
      if (!_test_different_procs || comm().size() == 1 || processor_id() == 1)
        flagInvalidSolution("Extra invalid thing!");
    }
  }
  _diffusivity[_qp] = _test_diffusivity;
}
