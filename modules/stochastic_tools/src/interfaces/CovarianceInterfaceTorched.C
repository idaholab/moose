//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CovarianceInterfaceTorched.h"
#include "CovarianceFunctionBaseTorched.h"

InputParameters
CovarianceInterfaceTorched::validParams()
{
  return emptyInputParameters();
}

CovarianceInterfaceTorched::CovarianceInterfaceTorched(const InputParameters & parameters)
  : _covar_feproblem(*parameters.get<FEProblemBase *>("_fe_problem_base"))
{
}

CovarianceFunctionBaseTorched *
CovarianceInterfaceTorched::getCovarianceFunctionByName(const UserObjectName & name) const
{
  std::vector<CovarianceFunctionBaseTorched *> models;
  _covar_feproblem.theWarehouse()
      .query()
      .condition<AttribName>(name)
      .condition<AttribSystem>("CovarianceFunctionTorched")
      .queryInto(models);
  if (models.empty())
    mooseError("Unable to find a CovarianceFunction object with the name '" + name + "'");
  return models[0];
}
