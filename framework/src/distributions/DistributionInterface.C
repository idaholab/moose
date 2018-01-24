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

template <>
InputParameters
validParams<DistributionInterface>()
{
  return emptyInputParameters();
}

DistributionInterface::DistributionInterface(const MooseObject * moose_object)
  : _dni_params(moose_object->parameters()),
    _dni_feproblem(*_dni_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
}

Distribution &
DistributionInterface::getDistribution(const std::string & name)
{
  DistributionName dist_name = _dni_params.get<DistributionName>(name);
  return _dni_feproblem.getDistribution(dist_name);
}

Distribution &
DistributionInterface::getDistributionByName(const DistributionName & name)
{
  return _dni_feproblem.getDistribution(name);
}
