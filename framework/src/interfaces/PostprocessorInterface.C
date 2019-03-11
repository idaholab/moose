//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorInterface.h"
#include "FEProblem.h"
#include "Postprocessor.h"
#include "MooseTypes.h"
#include "MooseObject.h"

PostprocessorInterface::PostprocessorInterface(const MooseObject * moose_object)
  : _ppi_params(moose_object->parameters()),
    _pi_feproblem(*_ppi_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValue(const std::string & name)
{
  // Return the default if the Postprocessor does not exist and a default does, otherwise
  // continue as usual
  if (!hasPostprocessor(name) && _ppi_params.hasDefaultPostprocessorValue(name))
    return _ppi_params.getDefaultPostprocessorValue(name);
  else
    return _pi_feproblem.getPostprocessorValue(_ppi_params.get<PostprocessorName>(name));
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOld(const std::string & name)
{
  // Return the default if the Postprocessor does not exist and a default does, otherwise
  // continue as usual
  if (!hasPostprocessor(name) && _ppi_params.hasDefaultPostprocessorValue(name))
    return _ppi_params.getDefaultPostprocessorValue(name);
  else
    return _pi_feproblem.getPostprocessorValueOld(_ppi_params.get<PostprocessorName>(name));
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOlder(const std::string & name)
{
  // Return the default if the Postprocessor does not exist and a default does, otherwise
  // continue as usual
  if (!hasPostprocessor(name) && _ppi_params.hasDefaultPostprocessorValue(name))
    return _ppi_params.getDefaultPostprocessorValue(name);
  else
    return _pi_feproblem.getPostprocessorValueOlder(_ppi_params.get<PostprocessorName>(name));
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueByName(const PostprocessorName & name)
{
  return _pi_feproblem.getPostprocessorValue(name);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOldByName(const PostprocessorName & name)
{
  return _pi_feproblem.getPostprocessorValueOld(name);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOlderByName(const PostprocessorName & name)
{
  return _pi_feproblem.getPostprocessorValueOlder(name);
}

bool
PostprocessorInterface::hasPostprocessor(const std::string & name) const
{
  return _pi_feproblem.hasPostprocessor(_ppi_params.get<PostprocessorName>(name));
}

bool
PostprocessorInterface::hasPostprocessorByName(const PostprocessorName & name)
{
  return _pi_feproblem.hasPostprocessor(name);
}

const PostprocessorValue &
PostprocessorInterface::getDefaultPostprocessorValue(const std::string & name)
{
  return _ppi_params.getDefaultPostprocessorValue(name);
}
