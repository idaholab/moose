//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInterpolationMethodInterface.h"

#include "FVInterpolationMethod.h"
#include "FEProblemBase.h"

InputParameters
FVInterpolationMethodInterface::validParams()
{
  return emptyInputParameters();
}

FVInterpolationMethodInterface::FVInterpolationMethodInterface(const MooseObject * moose_object)
  : _fvim_params(moose_object->parameters()),
    _fvim_feproblem(*_fvim_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _fvim_tid(_fvim_params.have_parameter<THREAD_ID>("_tid") ? _fvim_params.get<THREAD_ID>("_tid")
                                                             : 0)
{
}

const FVInterpolationMethod &
FVInterpolationMethodInterface::getFVInterpolationMethod(const InterpolationMethodName & name) const
{
  return _fvim_feproblem.getFVInterpolationMethod(name, _fvim_tid);
}

bool
FVInterpolationMethodInterface::hasFVInterpolationMethod(const InterpolationMethodName & name) const
{
  return _fvim_feproblem.hasFVInterpolationMethod(name);
}
