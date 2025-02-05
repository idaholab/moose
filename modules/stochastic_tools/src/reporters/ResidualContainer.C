//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Moose.h"
#include "MooseTypes.h"
#include "NonlinearSystem.h"
#include "ResidualContainer.h"
#include "NonlinearSystemBase.h"

registerMooseObject("StochasticToolsApp", ResidualContainer);

InputParameters
ResidualContainer::validParams()
{
  InputParameters params = SnapshotContainerBase::validParams();
  params.addClassDescription(
      "Class responsible for collecting distributed residual vectors into a container. We append "
      "a new distributed residual vector at every execution.");
  params.addRequiredParam<TagName>(
      "tag_name",
      "Name of the residual tag to collect snapshot, defaults to the total "
      "residual.");
  return params;
}

ResidualContainer::ResidualContainer(const InputParameters & parameters)
  : SnapshotContainerBase(parameters),
    _nl_sys(_fe_problem.getNonlinearSystem(_nonlinear_system_number)),
    _tag_id(_fe_problem.getVectorTagID(getParam<TagName>("tag_name")))
{
}

std::unique_ptr<NumericVector<Number>>
ResidualContainer::collectSnapshot()
{

  auto exec_flag = _fe_problem.getCurrentExecuteOnFlag();
  // The best time to get the residual for linear problems is at the beginning
  // of each timestep, but the residual is not calculated then. This forces it to run.
  if (exec_flag == EXEC_TIMESTEP_BEGIN)
    // This happens on timestep_begin
    _fe_problem.computeResidualL2Norm();

  return _nl_sys.getVector(_tag_id).clone();
}
