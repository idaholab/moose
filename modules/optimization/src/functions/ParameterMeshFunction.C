//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParameterMeshFunction.h"

#include "AddVariableAction.h"

registerMooseObject("OptimizationApp", ParameterMeshFunction);

InputParameters
ParameterMeshFunction::validParams()
{
  InputParameters params = OptimizationFunction::validParams();
  params.addClassDescription("Optimization function with parameters represented by a mesh and "
                             "finite-element shape functions.");

  params.addRequiredParam<FileName>("exodus_mesh", "File containing parameter mesh.");
  params.addParam<MooseEnum>("family",
                             AddVariableAction::getNonlinearVariableFamilies(),
                             "Family of FE shape functions for parameter.");
  params.addParam<MooseEnum>("order",
                             AddVariableAction::getNonlinearVariableOrders(),
                             "Order of FE shape functions for parameter.");

  params.addRequiredParam<ReporterName>(
      "parameter_name", "Reporter or VectorPostprocessor vector containing parameter values.");
  params.addParam<ReporterName>("time_name",
                                "Name of vector-postprocessor or reporter vector containing time, "
                                "default assumes time independence.");

  return params;
}

ParameterMeshFunction::ParameterMeshFunction(const InputParameters & parameters)
  : OptimizationFunction(parameters),
    ReporterInterface(this),
    _parameter_mesh(AddVariableAction::feType(parameters), getParam<FileName>("exodus_mesh")),
    _values(getReporterValue<std::vector<Real>>("parameter_name")),
    _coordt(isParamValid("time_name") ? getReporterValue<std::vector<Real>>("time_name")
                                      : _empty_vec)
{
}

Real
ParameterMeshFunction::value(Real t, const Point & p) const
{
  checkSize();

  const auto ti = interpolateTime(t);
  const dof_id_type offset0 = ti[0].first * _parameter_mesh.size();
  const dof_id_type offset1 = ti[1].first * _parameter_mesh.size();

  std::vector<dof_id_type> dof_indices;
  std::vector<Real> weights;
  _parameter_mesh.getIndexAndWeight(p, dof_indices, weights);

  Real val = 0;
  for (const auto & i : index_range(dof_indices))
    val += (_values[dof_indices[i] + offset0] * ti[0].second +
            _values[dof_indices[i] + offset1] * ti[1].second) *
           weights[i];
  return val;
}

RealGradient
ParameterMeshFunction::gradient(Real t, const Point & p) const
{
  checkSize();

  const auto ti = interpolateTime(t);
  const dof_id_type offset0 = ti[0].first * _parameter_mesh.size();
  const dof_id_type offset1 = ti[1].first * _parameter_mesh.size();

  std::vector<dof_id_type> dof_indices;
  std::vector<RealGradient> weights;
  _parameter_mesh.getIndexAndWeight(p, dof_indices, weights);

  RealGradient val(0, 0, 0);
  for (const auto & i : index_range(dof_indices))
    val += (_values[dof_indices[i] + offset0] * ti[0].second +
            _values[dof_indices[i] + offset1] * ti[1].second) *
           weights[i];
  return val;
}

Real
ParameterMeshFunction::timeDerivative(Real t, const Point & p) const
{
  checkSize();

  const auto ti = interpolateTime(t, true);
  if (ti[0].first == ti[1].first)
    return 0.0;
  const dof_id_type offset0 = ti[0].first * _parameter_mesh.size();
  const dof_id_type offset1 = ti[1].first * _parameter_mesh.size();

  std::vector<dof_id_type> dof_indices;
  std::vector<Real> weights;
  _parameter_mesh.getIndexAndWeight(p, dof_indices, weights);

  Real val = 0;
  for (const auto & i : index_range(dof_indices))
    val += (_values[dof_indices[i] + offset0] * ti[0].second +
            _values[dof_indices[i] + offset1] * ti[1].second) *
           weights[i];
  return val;
}

std::vector<Real>
ParameterMeshFunction::parameterGradient(Real t, const Point & p) const
{
  const auto ti = interpolateTime(t);
  const dof_id_type offset0 = ti[0].first * _parameter_mesh.size();
  const dof_id_type offset1 = ti[1].first * _parameter_mesh.size();

  std::vector<dof_id_type> dof_indices;
  std::vector<Real> weights;
  _parameter_mesh.getIndexAndWeight(p, dof_indices, weights);

  dof_id_type sz = _parameter_mesh.size();
  if (!_coordt.empty())
    sz *= _coordt.size();
  std::vector<Real> pg(sz, 0.0);
  for (const auto & i : index_range(dof_indices))
  {
    pg[dof_indices[i] + offset0] += weights[i] * ti[0].second;
    pg[dof_indices[i] + offset1] += weights[i] * ti[1].second;
  }
  return pg;
}

std::array<std::pair<std::size_t, Real>, 2>
ParameterMeshFunction::interpolateTime(Real t, bool derivative) const
{
  std::array<std::pair<std::size_t, Real>, 2> ti;
  if (_coordt.size() <= 1 || MooseUtils::absoluteFuzzyLessEqual(t, _coordt[0]))
  {
    ti[0] = {0, 0.5};
    ti[1] = {0, 0.5};
  }
  else if (MooseUtils::absoluteFuzzyGreaterEqual(t, _coordt.back()))
  {
    ti[0] = {_coordt.size() - 1, 0.5};
    ti[1] = {_coordt.size() - 1, 0.5};
  }
  else
    for (std::size_t i = 1; i < _coordt.size(); ++i)
      if (MooseUtils::absoluteFuzzyGreaterEqual(_coordt[i], t))
      {
        const Real dt = _coordt[i] - _coordt[i - 1];
        if (derivative)
        {
          ti[0] = {i - 1, -1.0 / dt};
          ti[1] = {i, 1.0 / dt};
        }
        else
        {
          ti[0] = {i - 1, (_coordt[i] - t) / dt};
          ti[1] = {i, (t - _coordt[i - 1]) / dt};
        }
        break;
      }

  return ti;
}

void
ParameterMeshFunction::checkSize() const
{
  dof_id_type sz = _parameter_mesh.size();
  if (!_coordt.empty())
    sz *= _coordt.size();
  if (sz != _values.size())
    paramError("parameter_name",
               "Size of parameter vector (",
               _values.size(),
               ") does not match number of degrees of freedom in mesh (",
               sz,
               ").");
}
