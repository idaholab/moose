//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVGradientInterface.h"

#include "SystemBase.h"
#include "MooseVariableFieldBase.h"
#include "MooseError.h"

#include "libmesh/numeric_vector.h"

using namespace libMesh;

bool
LinearFVGradientInterface::needsLinearFVGradientStorage() const
{
  for (const auto * const field_var : _sys.variableWarehouse().fieldVariables())
    if (field_var->needsGradientVectorStorage())
      return true;

  return false;
}

void
LinearFVGradientInterface::initializeContainer(
    std::vector<std::unique_ptr<NumericVector<Number>>> & container) const
{
  container.clear();
  mooseAssert(_sys.currentSolution(),
              "Current solution must exist before building FV gradient storage.");
  for (unsigned int i = 0; i < _sys.mesh().dimension(); ++i)
    container.push_back(_sys.currentSolution()->zero_clone());
}

void
LinearFVGradientInterface::rebuildLinearFVGradientStorage()
{
  _raw_grad_container.clear();
  _temporary_gradient.clear();
  _raw_limited_grad_containers.clear();
  _temporary_limited_gradient.clear();

  if (!needsLinearFVGradientStorage())
    return;

  initializeContainer(_raw_grad_container);
  initializeContainer(_temporary_gradient);

  for (const auto limiter_type : _requested_limited_gradient_types)
  {
    if (limiter_type == Moose::FV::GradientLimiterType::None)
      continue;

    initializeContainer(_raw_limited_grad_containers[limiter_type]);
    initializeContainer(_temporary_limited_gradient[limiter_type]);
  }
}

void
LinearFVGradientInterface::requestLinearFVLimitedGradients(
    const Moose::FV::GradientLimiterType limiter_type, const unsigned int variable_number)
{
  if (limiter_type == Moose::FV::GradientLimiterType::None)
    return;

  auto * const variable =
      dynamic_cast<MooseVariableFieldBase *>(_sys.variableWarehouse().getVariable(variable_number));
  if (!variable)
    mooseError("Limited gradients were requested for variable number ",
               variable_number,
               " on system '",
               _sys.name(),
               "', but no field variable with that number exists on the system.");

  if (!variable->needsGradientVectorStorage())
    mooseError("Limited gradients were requested for variable '",
               variable->name(),
               "' on system '",
               _sys.name(),
               "', but regular gradients were not requested for that variable.");

  _requested_limited_gradient_variables[limiter_type].insert(variable_number);

  if (_requested_limited_gradient_types.insert(limiter_type).second && !_raw_grad_container.empty())
  {
    initializeContainer(_raw_limited_grad_containers[limiter_type]);
    initializeContainer(_temporary_limited_gradient[limiter_type]);
  }
}

const std::vector<std::unique_ptr<NumericVector<Number>>> &
LinearFVGradientInterface::linearFVLimitedGradientContainer(
    const Moose::FV::GradientLimiterType limiter_type) const
{
  if (limiter_type == Moose::FV::GradientLimiterType::None)
    return _raw_grad_container;

  const auto it = _raw_limited_grad_containers.find(limiter_type);
  if (it == _raw_limited_grad_containers.end())
    mooseError("Limited gradient container was requested but not initialized on system '",
               _sys.name(),
               "'.");

  return it->second;
}
