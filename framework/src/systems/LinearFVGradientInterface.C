//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVGradientInterface.h"

#include "ComputeLinearFVGreenGaussGradientFaceThread.h"
#include "ComputeLinearFVGreenGaussGradientVolumeThread.h"
#include "ComputeLinearFVLimitedGradientThread.h"
#include "FEProblemBase.h"
#include "PerfGraphInterface.h"
#include "PerfGuard.h"
#include "SystemBase.h"
#include "MooseVariableFieldBase.h"
#include "MooseError.h"

#include "libmesh/numeric_vector.h"

using namespace libMesh;

void
LinearFVGradientInterface::computeGradients()
{
  // No gradients have been requested, by now we should have set up the
  // containers to receive the gradients. Time to ealry return.
  if (_raw_grad_container.empty())
    return;

  auto & temporary_gradient = temporaryLinearFVGradientContainer();
  mooseAssert(temporary_gradient.size() == _raw_grad_container.size(),
              "Temporary and raw gradient containers must have the same size.");
  for (auto & vec : temporary_gradient)
    vec->zero();

  auto & fe_problem = _sys.feProblem();
  auto * const perf_graph_interface = dynamic_cast<PerfGraphInterface *>(&_sys);
  mooseAssert(perf_graph_interface,
              "LinearFVGradientInterface requires its owning system to implement "
              "PerfGraphInterface.");
  const auto perf_id = perf_graph_interface->registerTimedSection("LinearVariableFV_Gradients", 3);
  mooseAssert(!Threads::in_threads, "PerfGraph timing cannot be used within threaded sections");
  PerfGuard time_guard(perf_graph_interface->perfGraph(), perf_id);

  PARALLEL_TRY
  {
    using FaceInfoRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    FaceInfoRange face_info_range(fe_problem.mesh().ownedFaceInfoBegin(),
                                  fe_problem.mesh().ownedFaceInfoEnd());

    ComputeLinearFVGreenGaussGradientFaceThread gradient_face_thread(
        fe_problem, _sys, temporary_gradient);
    Threads::parallel_reduce(face_info_range, gradient_face_thread);
  }
  fe_problem.checkExceptionAndStopSolve();

  for (auto & vec : temporary_gradient)
    vec->close();

  PARALLEL_TRY
  {
    using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
    ElemInfoRange elem_info_range(fe_problem.mesh().ownedElemInfoBegin(),
                                  fe_problem.mesh().ownedElemInfoEnd());

    ComputeLinearFVGreenGaussGradientVolumeThread gradient_volume_thread(
        fe_problem, _sys, temporary_gradient);
    Threads::parallel_reduce(elem_info_range, gradient_volume_thread);
  }
  fe_problem.checkExceptionAndStopSolve();

  for (const auto i : index_range(_raw_grad_container))
    temporary_gradient[i]->close();

  _raw_grad_container.swap(temporary_gradient);

  if (!requestedLinearFVLimitedGradientTypes().empty())
  {
    using ElemInfoRange = StoredRange<MooseMesh::const_elem_info_iterator, const ElemInfo *>;
    ElemInfoRange elem_info_range(fe_problem.mesh().ownedElemInfoBegin(),
                                  fe_problem.mesh().ownedElemInfoEnd());

    for (const auto limiter_type : requestedLinearFVLimitedGradientTypes())
    {
      if (limiter_type == Moose::FV::GradientLimiterType::None)
        continue;

      auto & raw_container = rawLinearFVLimitedGradientContainer(limiter_type);
      auto & temporary_container = temporaryLinearFVLimitedGradientContainer(limiter_type);
      mooseAssert(temporary_container.size() == raw_container.size(),
                  "Temporary and raw limited gradient containers must have the same size.");
      for (auto & vec : temporary_container)
        vec->zero();

      PARALLEL_TRY
      {
        ComputeLinearFVLimitedGradientThread limited_gradient_thread(
            fe_problem,
            _sys,
            _raw_grad_container,
            temporary_container,
            limiter_type,
            requestedLinearFVLimitedGradientVariables(limiter_type));
        Threads::parallel_reduce(elem_info_range, limited_gradient_thread);
      }
      fe_problem.checkExceptionAndStopSolve();

      for (auto & vec : temporary_container)
        vec->close();

      raw_container.swap(temporary_container);
    }
  }
}

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
