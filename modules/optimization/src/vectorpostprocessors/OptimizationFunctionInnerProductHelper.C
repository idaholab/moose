//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationFunctionInnerProductHelper.h"

// MOOSE includes
#include "InputParameters.h"
#include "MooseError.h"
#include "FEProblemBase.h"

// Optimization includes
#include "OptimizationFunction.h"

InputParameters
OptimizationFunctionInnerProductHelper::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<FunctionName>("function", "Optimization function.");
  params.addParam<Real>(
      "reverse_time_end",
      0.0,
      "End time used for reversing the time integration when evaluating function derivative.");
  return params;
}

OptimizationFunctionInnerProductHelper::OptimizationFunctionInnerProductHelper(
    const InputParameters & parameters)
  : _ip_problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _function(dynamic_cast<const OptimizationFunction *>(&_ip_problem.getFunction(
        parameters.get<FunctionName>("function"), parameters.get<THREAD_ID>("_tid")))),
    _reverse_time_end(parameters.get<Real>("reverse_time_end")),
    _simulation_time(_ip_problem.getMooseApp().getStartTime())
{
  if (!_function)
    mooseError("Function requested by ",
               parameters.get<std::string>("_type"),
               " must be an OptimizationFunction.");
}

void
OptimizationFunctionInnerProductHelper::setCurrentTime(Real time, Real dt)
{
  // We are saving the integral for each time step.
  // So create or grab the vector if the time has changed or this is our first time here.
  _curr_time_ip = nullptr;
  for (auto & pr : _time_ip)
    if (MooseUtils::relativeFuzzyEqual(time, pr.first))
      _curr_time_ip = &pr.second;
  if (!_curr_time_ip)
  {
    _time_ip.emplace_back(time, std::vector<Real>());
    _curr_time_ip = &_time_ip.back().second;
  }
  _curr_time_ip->clear();

  _actual_time =
      MooseUtils::absoluteFuzzyEqual(_reverse_time_end, 0.0) ? time : _reverse_time_end - time + dt;
}

void
OptimizationFunctionInnerProductHelper::update(const Point & q_point, Real q_inner_product)
{
  if (!_curr_time_ip)
    mooseError("Internal error, 'setCurrentTime' needs to be called before calling 'update'.");

  const std::vector<Real> pg = _function->parameterGradient(_actual_time, q_point);
  _curr_time_ip->resize(std::max(pg.size(), _curr_time_ip->size()), 0.0);
  for (const auto & i : index_range(pg))
    (*_curr_time_ip)[i] += q_inner_product * pg[i];
}

void
OptimizationFunctionInnerProductHelper::add(const OptimizationFunctionInnerProductHelper & other)
{
  _curr_time_ip->resize(std::max(_curr_time_ip->size(), other._curr_time_ip->size()), 0.0);
  for (const auto & i : index_range(*other._curr_time_ip))
    (*_curr_time_ip)[i] += (*other._curr_time_ip)[i];
}

void
OptimizationFunctionInnerProductHelper::getVector(std::vector<Real> & result)
{
  std::size_t nvar = _curr_time_ip->size();
  _ip_problem.comm().max(nvar);
  _curr_time_ip->resize(nvar, 0.0);
  _ip_problem.comm().sum(*_curr_time_ip);

  if (!_ip_problem.isTransient())
    result = (*_curr_time_ip);
  else
  {
    // Make sure everything is the same size
    for (const auto & it : _time_ip)
      nvar = std::max(nvar, it.second.size());
    for (auto & it : _time_ip)
      it.second.resize(nvar);
    result.assign(nvar, 0.0);

    // Integrate in time
    std::sort(_time_ip.begin(),
              _time_ip.end(),
              [](const std::pair<Real, std::vector<Real>> & a,
                 const std::pair<Real, std::vector<Real>> & b) { return a.first < b.first; });
    // We are integrating over the entire history here. Technically this can be done in an
    // accumulative manner by storing the integration over previous time steps. However, this
    // is a relatively cheap operation and considering all cases where time steps may be
    // repeated would be difficult.
    for (const auto & ti : _time_ip)
      for (const auto & i : make_range(nvar))
        result[i] += ti.second[i];
  }
}
