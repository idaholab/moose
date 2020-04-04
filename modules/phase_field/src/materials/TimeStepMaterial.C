//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeStepMaterial.h"

registerMooseObject("PhaseFieldApp", TimeStepMaterial);

InputParameters
TimeStepMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Provide various time stepping quantities as material properties.");
  params.addParam<MaterialPropertyName>(
      "prop_dt", "dt", "Material property to store the current dt");
  params.addParam<MaterialPropertyName>(
      "prop_time", "time", "Material property to store the current time");
  params.addParam<MaterialPropertyName>(
      "prop_time_step", "time_step", "Material property to store the current time step number");
  return params;
}

TimeStepMaterial::TimeStepMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop_dt(declareProperty<Real>(getParam<MaterialPropertyName>("prop_dt"))),
    _prop_time(declareProperty<Real>(getParam<MaterialPropertyName>("prop_time"))),
    _prop_time_step(declareProperty<Real>(getParam<MaterialPropertyName>("prop_time_step")))
{
}

void
TimeStepMaterial::computeQpProperties()
{
  _prop_dt[_qp] = _fe_problem.dt();
  _prop_time[_qp] = _fe_problem.time();
  _prop_time_step[_qp] = _fe_problem.timeStep();
}
