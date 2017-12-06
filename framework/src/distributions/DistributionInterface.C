/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
