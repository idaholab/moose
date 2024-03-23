//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SnapshotContainerBase.h"

InputParameters
SnapshotContainerBase::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addParam<NonlinearSystemName>(
      "nonlinear_system_name",
      "nl0",
      "Option to select which nonlinear system's solution shall be stored.");
  params.addParam<Real>("save_tolerance", 1e-8, "Tolerance for determining a duplicate snapshot.");
  return params;
}

SnapshotContainerBase::SnapshotContainerBase(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _accumulated_data(declareRestartableData<std::vector<std::unique_ptr<NumericVector<Number>>>>(
        "accumulated_snapshots")),
    _nonlinear_system_number(
        _fe_problem.nlSysNum(getParam<NonlinearSystemName>("nonlinear_system_name"))),
    _save_tolerance(getParam<Real>("save_tolerance"))
{
}

void
SnapshotContainerBase::initialSetup()
{
  _accumulated_data.clear();
}

const std::unique_ptr<NumericVector<Number>> &
SnapshotContainerBase::getSnapshot(unsigned int local_i) const
{
  mooseAssert(local_i < _accumulated_data.size(),
              "The container only has (" + std::to_string(_accumulated_data.size()) +
                  ") solutions so we cannot find any with index (" + std::to_string(local_i) +
                  ")!");
  return _accumulated_data[local_i];
}

void
SnapshotContainerBase::execute()
{
  auto possible_snap = collectSnapshot();
  // Do not store zero containers. These wouldn't hold any information anyway.
  if (possible_snap->linfty_norm() <= std::numeric_limits<Real>::epsilon())
    return;

  // We do this check every time we add a snap, so only need to check the last one.
  if (_accumulated_data.size() > 1)
    if (possible_snap->size() != _accumulated_data.back()->size())
      mooseError("Snapshot sizes changed mid-simulation.");

  // Store the cloned snapshot. Each derived class has to implement the collectSnapshot()
  // method.
  _accumulated_data.push_back(possible_snap->clone());
}
