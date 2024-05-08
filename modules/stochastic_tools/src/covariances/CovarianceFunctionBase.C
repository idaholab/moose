//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CovarianceFunctionBase.h"

InputParameters
CovarianceFunctionBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addClassDescription("Base class for covariance functions");
  params.registerBase("CovarianceFunctionBase");
  params.registerSystemAttributeName("CovarianceFunctionBase");
  return params;
}

CovarianceFunctionBase::CovarianceFunctionBase(const InputParameters & parameters)
  : MooseObject(parameters)
{
}

void
CovarianceFunctionBase::computedKdhyper(RealEigenMatrix & /*dKdhp*/,
                                        const RealEigenMatrix & /*x*/,
                                        std::string /*hyper_param_name*/,
                                        unsigned int /*ind*/) const
{
  mooseError("Hyperparameter tuning not set up for this covariance function. Please define "
             "computedKdhyper() to compute gradient.");
}

bool
CovarianceFunctionBase::isTunable(std::string name) const
{
  if (_tunable_hp.find(name) != _tunable_hp.end())
    return true;
  else if (isParamValid(name))
    mooseError("Tuning not supported for parameter ", name);
  else
    mooseError("Parameter ", name, " selected for tuning is not a valid parameter");
  return false;
}
