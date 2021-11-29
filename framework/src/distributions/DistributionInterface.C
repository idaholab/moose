//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DistributionInterface.h"
#include "Distribution.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "FEProblemBase.h"

InputParameters
DistributionInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

DistributionInterface::DistributionInterface(const MooseObject * moose_object)
  : _dni_params(moose_object->parameters()),
    _dni_feproblem(*_dni_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _dni_moose_object_ptr(moose_object)
{
}

const Distribution &
DistributionInterface::getDistribution(const std::string & name) const
{
  DistributionName dist_name = _dni_params.get<DistributionName>(name);
  return _dni_feproblem.getDistribution(dist_name);
}

const Distribution &
DistributionInterface::getDistributionByName(const DistributionName & name) const
{
  return _dni_feproblem.getDistribution(name);
}
