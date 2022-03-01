//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionInterface.h"
#include "Function.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "FEProblemBase.h"

InputParameters
FunctionInterface::validParams()
{
  return emptyInputParameters();
}

FunctionInterface::FunctionInterface(const MooseObject * moose_object)
  : _fni_params(moose_object->parameters()),
    _fni_feproblem(*_fni_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _fni_tid(_fni_params.have_parameter<THREAD_ID>("_tid") ? _fni_params.get<THREAD_ID>("_tid") : 0)
{
}

const Function &
FunctionInterface::getFunction(const std::string & name) const
{
  return _fni_feproblem.getFunction(_fni_params.get<FunctionName>(name), _fni_tid);
}

const Function &
FunctionInterface::getFunctionByName(const FunctionName & name) const
{
  return _fni_feproblem.getFunction(name, _fni_tid);
}

template <typename T>
const FunctionTempl<T> &
FunctionInterface::getFunction(const std::string & name) const
{
  return _fni_feproblem.getFunction<T>(_fni_params.get<FunctionName>(name), _fni_tid);
}

template <typename T>
const FunctionTempl<T> &
FunctionInterface::getFunctionByName(const FunctionName & name) const
{
  return _fni_feproblem.getFunction<T>(name, _fni_tid);
}

template const FunctionTempl<Real> &
FunctionInterface::getFunction<Real>(const std::string & name) const;
template const FunctionTempl<ADReal> &
FunctionInterface::getFunction<ADReal>(const std::string & name) const;
template const FunctionTempl<Real> &
FunctionInterface::getFunctionByName<Real>(const FunctionName & name) const;
template const FunctionTempl<ADReal> &
FunctionInterface::getFunctionByName<ADReal>(const FunctionName & name) const;
