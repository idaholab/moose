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

#include "SamplerInterface.h"
#include "Sampler.h"
#include "SubProblem.h"
#include "MooseTypes.h"

template <>
InputParameters
validParams<SamplerInterface>()
{
  return emptyInputParameters();
}

SamplerInterface::SamplerInterface(const MooseObject * moose_object)
  : _fni_params(moose_object->parameters()),
    _fni_feproblem(*_fni_params.get<FEProblemBase *>("_fe_problem_base")),
    _fni_tid(_fni_params.have_parameter<THREAD_ID>("_tid") ? _fni_params.get<THREAD_ID>("_tid") : 0)
{
}

Sampler &
SamplerInterface::getSampler(const std::string & name)
{
  SamplerName dist_name = _fni_params.get<SamplerName>(name);
  return _fni_feproblem.getSampler(dist_name, _fni_tid);
}

Sampler &
SamplerInterface::getSamplerByName(const SamplerName & name)
{
  return _fni_feproblem.getSampler(name, _fni_tid);
}
