//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BDF2.h"
#include "NonlinearSystem.h"
#include "FEProblemBase.h"

registerMooseObject("MooseApp", BDF2);

template <>
InputParameters
validParams<BDF2>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

BDF2::BDF2(const InputParameters & parameters)
  : TimeIntegrator(parameters), _weight(declareRestartableData<std::vector<Real>>("weight"))
{
  _weight.resize(3);
}

void
BDF2::preStep()
{
  if (_t_step > 1)
  {
    Real sum = _dt + _dt_old;
    _weight[0] = 1. + _dt / sum;
    _weight[1] = -sum / _dt_old;
    _weight[2] = _dt * _dt / _dt_old / sum;
  }
}

void
BDF2::computeTimeDerivatives()
{
  if (_t_step == 1)
  {
    _u_dot = *_solution;
    _u_dot -= _solution_old;
    _u_dot *= 1 / _dt;
    _u_dot.close();

    _du_dot_du = 1.0 / _dt;
  }
  else
  {
    _u_dot.zero();
    _u_dot.add(_weight[0], *_solution);
    _u_dot.add(_weight[1], _solution_old);
    _u_dot.add(_weight[2], _solution_older);
    _u_dot.scale(1. / _dt);
    _u_dot.close();

    _du_dot_du = _weight[0] / _dt;
  }
}

void
BDF2::computeDotProperties(Moose::MaterialDataType type, THREAD_ID tid) const
{
  auto material_data = _fe_problem.getMaterialData(type, tid);
  if (!material_data->hasDotProperties())
    return;

  const auto & props = material_data->props();
  const auto & old_props = material_data->propsOld();
  const auto & older_props = material_data->propsOlder();
  auto & dot_props = material_data->propsDot();

  auto n = dot_props.size();
  for (decltype(n) i = 0; i < n; ++i)
  {
    // skip properties that have not been required for time derivatives
    if (!dot_props[i])
      continue;

    auto & dot_prop = dynamic_cast<MaterialProperty<Real> &>(*dot_props[i]);
    auto & prop = dynamic_cast<const MaterialProperty<Real> &>(*props[i]);
    auto & old_prop = dynamic_cast<const MaterialProperty<Real> &>(*old_props[i]);
    auto & older_prop = dynamic_cast<const MaterialProperty<Real> &>(*older_props[i]);
    for (unsigned int qp = 0; qp < prop.size(); ++qp)
      dot_prop[qp] =
          (prop[qp] * _weight[0] + old_prop[qp] * _weight[1] + older_prop[qp] * _weight[2]) / _dt;
  }
}

void
BDF2::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual.close();
}
