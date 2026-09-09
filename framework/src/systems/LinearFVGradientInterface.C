//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVGradientManager.h"

#include "FEProblemBase.h"
#include "FVGradientMethod.h"
#include "MooseApp.h"
#include "PerfGraphInterface.h"
#include "PerfGuard.h"
#include "RestartableEquationSystems.h"
#include "SystemBase.h"
#include "MooseVariableFieldBase.h"
#include "MooseError.h"
#include "ElemInfo.h"
#include "FaceInfo.h"
#include "MathFVUtils.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/system.h"

using namespace libMesh;

namespace
{
template <typename DestinationContainer>
void
copyGradient(const LinearFVGradientReader::GradientContainer & source,
             DestinationContainer & destination)
{
  mooseAssert(!Threads::in_threads, "Linear FV gradient state copying is not thread-safe.");
  mooseAssert(source.size() == destination.size(),
              "Gradient state component counts must match when copying states.");

  for (const auto component : index_range(source))
  {
    mooseAssert(source[component], "Source gradient component vector must be initialized.");
    mooseAssert(destination[component],
                "Destination gradient component vector must be initialized.");
    *destination[component] = *source[component];
  }
}
}

const FVGradientMethod &
LinearFVGradientManager::resolveFVGradientMethod(const GradientMethodName & method_name)
{
  auto & fe_problem = _sys.feProblem();

  if (!fe_problem.hasFVGradientMethod(method_name))
  {
    if (method_name == "green-gauss")
    {
      auto params = fe_problem.getMooseApp().getFactory().getValidParams("FVGreenGaussGradient");
      fe_problem.addFVGradientMethod("FVGreenGaussGradient", method_name, params);
    }
    else if (method_name == "green-gauss-venkatakrishnan")
    {
      auto params = fe_problem.getMooseApp().getFactory().getValidParams("FVGreenGaussGradient");
      params.set<MooseEnum>("limiter") = "venkatakrishnan";
      fe_problem.addFVGradientMethod("FVGreenGaussGradient", method_name, params);
    }
  }

  if (!fe_problem.hasFVGradientMethod(method_name))
    mooseError("Unable to find FVGradientMethod with name '", method_name, "'");

  return fe_problem.getFVGradientMethod(method_name);
}

LinearFVGradientReader
LinearFVGradientManager::registerFVGradient(const unsigned int variable_number,
                                            const FVGradientMethod & method,
                                            const unsigned int oldest_state)
{
  auto * const variable =
      dynamic_cast<MooseVariableFieldBase *>(_sys.variableWarehouse().getVariable(variable_number));
  if (!variable)
    mooseError("Linear FV gradients were requested for variable number ",
               variable_number,
               " on system '",
               _sys.name(),
               "', but no field variable with that number exists on the system.");

  auto & container = _linear_fv_gradient_container_by_method[&method];
  container.variable_numbers.insert(variable_number);

  resizeGradientStateStorage(container, oldest_state);

  const auto & current_solution = _sys.system().current_local_solution;
  if (current_solution && current_solution->initialized())
  {
    if (container.current_values.empty())
      initializeContainer(container.current_values);
    if (container.next_values.empty())
      initializeContainer(container.next_values);
    setCurrentGradientState(container);
  }

  return LinearFVGradientReader(_sys, container.state_values, method, variable_number);
}

void
LinearFVGradientManager::computeGradients()
{
  if (_linear_fv_gradient_container_by_method.empty() || !_sys.solutionStatesInitialized())
    return;

  auto * const perf_graph_interface = dynamic_cast<PerfGraphInterface *>(&_sys);
  mooseAssert(perf_graph_interface,
              "LinearFVGradientManager requires its owning system to implement "
              "PerfGraphInterface.");
  const auto perf_id = perf_graph_interface->registerTimedSection("LinearVariableFV_Gradients", 3);
  mooseAssert(!Threads::in_threads, "PerfGraph timing cannot be used within threaded sections");
  PerfGuard time_guard(perf_graph_interface->perfGraph(), perf_id);

  // Keep current values unchanged until every replacement has been computed so boundary
  // conditions consistently use gradients from the previous update.
  // BCs may use cell gradients to compute the boundary face value, which is itself used to
  // compute cell gradients
  for (auto & method_container_pair : _linear_fv_gradient_container_by_method)
    computeLinearFVGradientContainer(*method_container_pair.first);

  for (auto & method_container_pair : _linear_fv_gradient_container_by_method)
    finalizeLinearFVGradientContainer(method_container_pair.second);
}

void
LinearFVGradientManager::updateFVGradient(const LinearFVGradientReader & reader)
{
  if (&reader.system() != &_sys)
    mooseError("Requested update for a linear FV gradient field from a different system than '",
               _sys.name(),
               "'.");

  const auto method_container_pair = _linear_fv_gradient_container_by_method.find(&reader.method());
  if (method_container_pair != _linear_fv_gradient_container_by_method.end())
  {
    auto * const perf_graph_interface = dynamic_cast<PerfGraphInterface *>(&_sys);
    mooseAssert(perf_graph_interface,
                "LinearFVGradientManager requires its owning system to implement "
                "PerfGraphInterface.");
    const auto perf_id =
        perf_graph_interface->registerTimedSection("LinearVariableFV_Gradients", 3);
    mooseAssert(!Threads::in_threads, "PerfGraph timing cannot be used within threaded sections");
    PerfGuard time_guard(perf_graph_interface->perfGraph(), perf_id);

    auto & container = computeLinearFVGradientContainer(reader.method());
    finalizeLinearFVGradientContainer(container);
    return;
  }

  mooseError("Requested update for an unregistered linear FV gradient field on system '",
             _sys.name(),
             "'.");
}

bool
LinearFVGradientManager::hasLinearFVGradients() const
{
  return !_linear_fv_gradient_container_by_method.empty();
}

void
LinearFVGradientManager::initializeContainer(OwnedGradientContainer & container) const
{
  container.clear();
  const auto & current_solution = _sys.system().current_local_solution;
  mooseAssert(current_solution && current_solution->initialized(),
              "Current solution must exist before building FV gradient storage.");
  container.resize(_sys.mesh().dimension());
  for (auto & component : container)
    component = current_solution->zero_clone();
}

void
LinearFVGradientManager::resizeGradientStateStorage(LinearFVGradientContainer & container,
                                                    const unsigned int oldest_state)
{
  const auto required_states = static_cast<std::size_t>(oldest_state) + 1;
  const auto old_size = container.state_values.size();
  if (required_states > old_size && _sys.solutionStatesInitialized() && oldest_state > 0)
    mooseError("Linear FV gradient state ",
               oldest_state,
               " was requested on system '",
               _sys.name(),
               "' after solution states were initialized. Old gradient states must be requested "
               "during setup.");

  if (required_states > old_size)
    container.state_values.resize(required_states);
}

void
LinearFVGradientManager::initializeLinearFVGradientHistoryStorage()
{
  for (auto & [method, container] : _linear_fv_gradient_container_by_method)
    for (const auto state : make_range(std::size_t(1), container.state_values.size()))
      if (container.state_values[state].empty())
      {
        GradientContainer state_values(_sys.mesh().dimension());
        for (const auto component : index_range(state_values))
          state_values[component] = &_sys.addVector(
              gradientStateVectorName(*method, state, component), true, libMesh::GHOSTED);
        container.state_values[state] = std::move(state_values);
      }
}

void
LinearFVGradientManager::setCurrentGradientState(LinearFVGradientContainer & container) const
{
  mooseAssert(!container.state_values.empty(),
              "Gradient state storage must contain a current state.");
  auto & current_view = container.state_values[0];
  current_view.clear();
  current_view.reserve(container.current_values.size());
  for (auto & component : container.current_values)
    current_view.push_back(component.get());
}

std::string
LinearFVGradientManager::gradientStateVectorName(const FVGradientMethod & method,
                                                 const unsigned int state,
                                                 const unsigned int component)
{
  return "linear_fv_gradient_" + method.name() + "_state_" + std::to_string(state) + "_component_" +
         std::to_string(component);
}

void
LinearFVGradientManager::checkRestartedGradientHistory(const FVGradientMethod & method,
                                                       LinearFVGradientContainer & container)
{
  if (container.has_checked_restart_history)
    return;

  container.has_checked_restart_history = true;
  if (container.state_values.size() <= 1)
    return;

  const auto & app = _sys.feProblem().getMooseApp();
  if (!app.isRestarting() && !app.isRecovering())
    return;

  const auto & restartable_equation_systems = _sys.feProblem().getRestartableEquationSystems();
  for (const auto state : make_range(std::size_t(1), container.state_values.size()))
    for (const auto component : index_range(container.state_values[state]))
    {
      const auto vector_name = gradientStateVectorName(method, state, component);
      for (const auto variable_number : container.variable_numbers)
      {
        const auto & variable = _sys.getVariable(0, variable_number);
        if (!restartable_equation_systems.isVariableRestored(
                _sys.name(), vector_name, variable.name()))
          mooseError("Linear FV gradient state ",
                     state,
                     " for variable '",
                     variable.name(),
                     "' using gradient method '",
                     method.name(),
                     "' was requested on system '",
                     _sys.name(),
                     "', but vector '",
                     vector_name,
                     "' was not available in the restart data.");
      }
    }

  container.has_initialized_history = true;
}

LinearFVGradientManager::LinearFVGradientContainer &
LinearFVGradientManager::computeLinearFVGradientContainer(const FVGradientMethod & method)
{
  // Gradient requests can be registered before the current solution vector exists, so the current
  // and replacement fields cannot always be created when the request is recorded. These fields
  // must be zero clones of the solution vector to inherit its finalized layout and parallel type.
  // Requests can also arrive after the normal initialization phase. Initializing immediately
  // before computation handles both cases, while the empty checks make repeated calls harmless.
  initializeLinearFVGradientStorage();

  auto & container = libmesh_map_find(_linear_fv_gradient_container_by_method, &method);

  mooseAssert(!container.state_values.empty(),
              "Gradient state storage must contain a current state.");
  mooseAssert(!container.state_values[0].empty(),
              "Gradient storage must be initialized before gradient computation.");
  mooseAssert(!container.next_values.empty(),
              "Replacement gradient storage must be initialized before gradient computation.");
  mooseAssert(container.next_values.size() == container.current_values.size(),
              "Next and current gradient containers must have the same size.");

  method.computeGradient(_sys, container.next_values, container.variable_numbers);

  return container;
}

void
LinearFVGradientManager::finalizeLinearFVGradientContainer(LinearFVGradientContainer & container)
{
  mooseAssert(!container.state_values.empty(),
              "Gradient state storage must contain a current state.");
  mooseAssert(container.next_values.size() == container.current_values.size(),
              "Next and current gradient containers must have the same size.");
  container.current_values.swap(container.next_values);
  setCurrentGradientState(container);

  if (!container.has_initialized_history)
  {
    for (const auto state : make_range(std::size_t(1), container.state_values.size()))
      copyGradient(container.state_values[0], container.state_values[state]);
    container.has_initialized_history = true;
  }

  container.has_computed_gradient = true;
}

void
LinearFVGradientManager::initializeLinearFVGradientStorage()
{
  for (auto & [method, container] : _linear_fv_gradient_container_by_method)
  {
    if (container.current_values.empty())
      initializeContainer(container.current_values);
    if (container.next_values.empty())
      initializeContainer(container.next_values);
    setCurrentGradientState(container);
    checkRestartedGradientHistory(*method, container);
  }
}

void
LinearFVGradientManager::rebuildLinearFVGradientStorage()
{
  for (auto & method_container_pair : _linear_fv_gradient_container_by_method)
  {
    method_container_pair.second.current_values.clear();
    method_container_pair.second.next_values.clear();
    method_container_pair.second.has_computed_gradient = false;
  }

  initializeLinearFVGradientStorage();
}

void
LinearFVGradientManager::initializeGradientStatesForTimeAdvance()
{
  for (auto & [method, container] : _linear_fv_gradient_container_by_method)
    if (container.state_values.size() > 1 && !container.has_computed_gradient)
      computeLinearFVGradientContainer(*method);

  for (auto & [_, container] : _linear_fv_gradient_container_by_method)
    if (container.state_values.size() > 1 && !container.has_computed_gradient)
      finalizeLinearFVGradientContainer(container);
}

void
LinearFVGradientManager::copyPreviousGradientStates(
    const Moose::SolutionIterationType iteration_type, const bool skip_current_to_old)
{
  if (iteration_type != Moose::SolutionIterationType::Time)
    return;

  mooseAssert(!Threads::in_threads, "Linear FV gradient state copying is not thread-safe.");
  initializeGradientStatesForTimeAdvance();

  for (auto & [_, container] : _linear_fv_gradient_container_by_method)
  {
    const auto number_of_states = container.state_values.size();
    if (number_of_states <= 1)
      continue;

    mooseAssert(container.has_computed_gradient,
                "Current gradient state must be initialized before advancing time states.");
    // Mirror solution-state advancement: some restore workflows preserve state 1 by skipping the
    // current-to-old copy while still shifting every deeper state.
    const std::size_t stop = skip_current_to_old ? 1 : 0;
    for (std::size_t state = number_of_states - 1; state > stop; --state)
      copyGradient(container.state_values[state - 1], container.state_values[state]);
  }
}

void
LinearFVGradientManager::restoreGradientStates()
{
  mooseAssert(!Threads::in_threads, "Linear FV gradient state copying is not thread-safe.");
  for (auto & [_, container] : _linear_fv_gradient_container_by_method)
  {
    if (container.state_values.size() <= 1)
      continue;

    mooseAssert(container.has_computed_gradient,
                "Current gradient state must be initialized before restoration.");
    copyGradient(container.state_values[1], container.state_values[0]);
    copyGradient(container.state_values[0], container.next_values);
  }
}
