//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LikelihoodInterface.h"
#include "Likelihood.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "FEProblemBase.h"

InputParameters
LikelihoodInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

LikelihoodInterface::LikelihoodInterface(const MooseObject * moose_object)
  : _dni_params(moose_object->parameters()),
    _dni_feproblem(*_dni_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _dni_moose_object_ptr(moose_object)
{
}

const Likelihood &
LikelihoodInterface::getLikelihood(const std::string & name) const
{
  LikelihoodName dist_name = _dni_params.get<LikelihoodName>(name);
  return _dni_feproblem.getLikelihood(dist_name);
}

const Likelihood &
LikelihoodInterface::getLikelihoodByName(const LikelihoodName & name) const
{
  return _dni_feproblem.getLikelihood(name);
}
