//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParallelAcquisitionInterface.h"
#include "ParallelAcquisitionFunctionBase.h"

InputParameters
ParallelAcquisitionInterface::validParams()
{
  return emptyInputParameters();
}

ParallelAcquisitionInterface::ParallelAcquisitionInterface(const InputParameters & parameters)
  : _parallelacquisition_feproblem(*parameters.get<FEProblemBase *>("_fe_problem_base"))
{
}

ParallelAcquisitionFunctionBase *
ParallelAcquisitionInterface::getParallelAcquisitionFunctionByName(const UserObjectName & name) const
{
  std::vector<ParallelAcquisitionFunctionBase *> models;
  _parallelacquisition_feproblem.theWarehouse()
      .query()
      .condition<AttribName>(name)
      .condition<AttribSystem>("ParallelAcquisitionFunction")
      .queryInto(models);

  if (models.empty())
    mooseError("Unable to find a ParallelAcquisitionFunction object with the name '" + name + "'");
  return models[0];
}
