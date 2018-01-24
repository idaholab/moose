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

template <>
InputParameters
validParams<SamplerInterface>()
{
  return emptyInputParameters();
}

SamplerInterface::SamplerInterface(const MooseObject * moose_object)
  : _smi_params(moose_object->parameters()),
    _smi_feproblem(*_smi_params.get<FEProblemBase *>("_fe_problem_base")),
    _smi_tid(_smi_params.have_parameter<THREAD_ID>("_tid") ? _smi_params.get<THREAD_ID>("_tid") : 0)
{
}

Sampler &
SamplerInterface::getSampler(const std::string & name)
{
  return _smi_feproblem.getSampler(_smi_params.get<SamplerName>(name), _smi_tid);
}

Sampler &
SamplerInterface::getSamplerByName(const SamplerName & name)
{
  return _smi_feproblem.getSampler(name, _smi_tid);
}
