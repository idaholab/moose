//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SamplerInterface.h"
#include "Sampler.h"
#include "SubProblem.h"
#include "MooseTypes.h"

InputParameters
SamplerInterface::validParams()
{
  return emptyInputParameters();
}

SamplerInterface::SamplerInterface(const MooseObject * moose_object)
  : _si_params(moose_object->parameters()),
    _si_feproblem(*_si_params.get<FEProblemBase *>("_fe_problem_base")),
    _si_tid(_si_params.have_parameter<THREAD_ID>("_tid") ? _si_params.get<THREAD_ID>("_tid") : 0)
{
}

template <>
Sampler &
SamplerInterface::getSampler(const std::string & name)
{
  return _si_feproblem.getSampler(_si_params.get<SamplerName>(name));
}

template <>
Sampler &
SamplerInterface::getSamplerByName(const SamplerName & name)
{
  return _si_feproblem.getSampler(name, _si_tid);
}
