//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LikelihoodInterface.h"

InputParameters
LikelihoodInterface::validParams()
{
  return emptyInputParameters();
}

LikelihoodInterface::LikelihoodInterface(const InputParameters & parameters)
  : _likelihood_feproblem(*parameters.get<FEProblemBase *>("_fe_problem_base"))
{
}

LikelihoodFunctionBase *
LikelihoodInterface::getLikelihoodFunctionByName(const UserObjectName & name) const
{
  std::vector<LikelihoodFunctionBase *> models;
  _likelihood_feproblem.theWarehouse()
      .query()
      .condition<AttribName>(name)
      .condition<AttribSystem>("LikelihoodFunctionBase")
      .queryInto(models);

  if (models.empty())
    mooseError("Unable to find a LikelihoodFunction object with the name '" + name + "'");
  return models[0];
}
