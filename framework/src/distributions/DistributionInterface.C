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
  : _fni_params(moose_object->parameters()),
    _fni_feproblem(*_fni_params.get<FEProblemBase *>("_fe_problem_base")),
    _fni_tid(_fni_params.have_parameter<THREAD_ID>("_tid") ? _fni_params.get<THREAD_ID>("_tid") : 0)
{
}

Distribution &
DistributionInterface::getDistribution(const std::string & name)
{
  DistributionName dist_name = _fni_params.get<DistributionName>(name);
  return _fni_feproblem.getDistribution(dist_name, _fni_tid);
}

Distribution &
DistributionInterface::getDistributionByName(const DistributionName & name)
{
  return _fni_feproblem.getDistribution(name, _fni_tid);
}
