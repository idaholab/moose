//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrankNicolson.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"

registerMooseObject("MooseApp", CrankNicolson);

template <>
InputParameters
validParams<CrankNicolson>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

CrankNicolson::CrankNicolson(const InputParameters & parameters)
  : TimeIntegrator(parameters), _residual_old(_nl.addVector("residual_old", false, GHOSTED))
{
}

void
CrankNicolson::computeTimeDerivatives()
{
  _u_dot = *_solution;
  _u_dot -= _solution_old;
  _u_dot *= 2. / _dt;
  _u_dot.close();

  _du_dot_du = 2. / _dt;
}

void
CrankNicolson::computeDotProperties(Moose::MaterialDataType type, THREAD_ID tid) const
{
  auto material_data = _fe_problem.getMaterialData(type, tid);
  if (!material_data->hasDotProperties())
    return;

  const auto & props = material_data->props();
  const auto & old_props = material_data->propsOld();
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
    for (unsigned int qp = 0; qp < prop.size(); ++qp)
      dot_prop[qp] = (prop[qp] - old_prop[qp]) / _dt * 2;
  }
}

void
CrankNicolson::init()
{
  // time derivative is assumed to be zero on initial
  _u_dot.zero();
  _du_dot_du = 0;

  // compute residual for the initial time step
  // Note: we can not directly pass _residual_old in computeResidualType because
  //       the function will call postResidual, which will cause _residual_old
  //       to be added on top of itself prohibited by PETSc.
  //       Objects executed on initial have been executed by FEProblem,
  //       so we can and should directly call NonlinearSystem residual evaluation.
  _nl.computeResidualTag(_nl.RHS(), _nl.nonTimeVectorTag());
  _residual_old = _nl.RHS();
}

void
CrankNicolson::postResidual(NumericVector<Number> & residual)
{
  residual += _Re_time;
  residual += _Re_non_time;
  residual += _residual_old;
}

void
CrankNicolson::postStep()
{
  // shift the residual in time
  _residual_old = _Re_non_time;
}
