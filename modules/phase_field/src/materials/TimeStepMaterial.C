/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "TimeStepMaterial.h"

template <>
InputParameters
validParams<TimeStepMaterial>()
{
  InputParameters params = validParams<Material>();
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
