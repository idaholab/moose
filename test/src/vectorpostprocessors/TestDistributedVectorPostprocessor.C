//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestDistributedVectorPostprocessor.h"

registerMooseObject("MooseTestApp", TestDistributedVectorPostprocessor);

InputParameters
TestDistributedVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  return params;
}

TestDistributedVectorPostprocessor::TestDistributedVectorPostprocessor(
    const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), _data(declareVector("data"))
{
}

void
TestDistributedVectorPostprocessor::execute()
{
  const std::size_t size = 10;
  const dof_id_type rank = processor_id();
  _data.resize(size);
  for (std::size_t i = 0; i < size; i++)
    _data[i] = size * size * (rank + 1) + i; // 100, 101, ..., 200, 201, ...
}

void
TestDistributedVectorPostprocessor::finalize()
{
  if (!isDistributed())
    _communicator.gather(0, _data);
}
