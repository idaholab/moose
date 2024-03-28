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
  return params;
}

SnapshotContainerBase::SnapshotContainerBase(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _accumulated_data(
        declareRestartableDataWithContext<Snapshots>("accumulated_snapshots", (void *)&comm())),
    _nonlinear_system_number(
        _fe_problem.nlSysNum(getParam<NonlinearSystemName>("nonlinear_system_name")))
{
}

void
SnapshotContainerBase::initialSetup()
{
  _accumulated_data.clear();
}

const NumericVector<Number> &
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
  // Store the cloned snapshot. Each derived class has to implement the cloneSnapshot() method.
  _accumulated_data.addPointer(collectSnapshot());
}

void
dataStore(std::ostream & stream, SnapshotContainerBase::Snapshots & v, void * context)
{
  storeHelper(stream, static_cast<UniqueStorage<NumericVector<Number>> &>(v), context);
}

void
dataLoad(std::istream & stream, SnapshotContainerBase::Snapshots & v, void * context)
{
  loadHelper(stream, static_cast<UniqueStorage<NumericVector<Number>> &>(v), context);
}
