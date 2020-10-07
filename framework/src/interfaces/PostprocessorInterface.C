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
#include "UserObject.h"

PostprocessorInterface::PostprocessorInterface(const MooseObject * moose_object)
  : _ppi_params(moose_object->parameters()),
    _pi_feproblem(*_ppi_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValue(const std::string & name, unsigned int index) const
{
  return getPostprocessorValueHelper(name, index, 0);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOld(const std::string & name, unsigned int index) const
{
  return getPostprocessorValueHelper(name, index, 1);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOlder(const std::string & name,
                                                   unsigned int index) const
{
  return getPostprocessorValueHelper(name, index, 2);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueByName(const PostprocessorName & name) const
{
  return getPostprocessorValueByNameHelper(name, 0);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOldByName(const PostprocessorName & name) const
{
  return getPostprocessorValueByNameHelper(name, 1);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueOlderByName(const PostprocessorName & name) const
{
  return getPostprocessorValueByNameHelper(name, 2);
}

bool
PostprocessorInterface::hasPostprocessor(const std::string & name, unsigned int index) const
{
  if (singlePostprocessor(name))
    return hasPostprocessorByName(_ppi_params.get<PostprocessorName>(name));
  return hasPostprocessorByName(_ppi_params.get<std::vector<PostprocessorName>>(name)[index]);
}

unsigned int
PostprocessorInterface::coupledPostprocessors(const std::string & name) const
{
  if (singlePostprocessor(name))
    return 1;
  return _ppi_params.get<std::vector<PostprocessorName>>(name).size();
}

bool
PostprocessorInterface::singlePostprocessor(const std::string & name) const
{
  return _ppi_params.isSinglePostprocessor(name);
}

bool
PostprocessorInterface::hasPostprocessorByName(const PostprocessorName & name) const
{
  ReporterName r_name(name, "value");
  return _pi_feproblem.getReporterData().hasReporterValue<PostprocessorValue>(r_name);
}

bool
PostprocessorInterface::hasPostprocessorObject(const std::string & name, unsigned int index) const
{
  if (singlePostprocessor(name))
    return hasPostprocessorObjectByName(_ppi_params.get<PostprocessorName>(name));
  return hasPostprocessorObjectByName(_ppi_params.get<std::vector<PostprocessorName>>(name)[index]);
}

bool
PostprocessorInterface::hasPostprocessorObjectByName(const PostprocessorName & name) const
{
  if (_pi_feproblem.hasUserObject(name))
  {
    const UserObject & uo = _pi_feproblem.getUserObjectBase(name);
    return dynamic_cast<const Postprocessor *>(&uo) != nullptr;
  }
  return false;
}

const PostprocessorValue &
PostprocessorInterface::getDefaultPostprocessorValue(const std::string & name) const
{
  return _ppi_params.getDefaultPostprocessorValue(name);
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueHelper(const std::string & name,
                                                    unsigned int index,
                                                    std::size_t t_index) const
{
  if (!_ppi_params.isParamValid(name))
    mooseError("The supplied Postprocessor name(s) parameter '", name, "' is not defined.");

  if (singlePostprocessor(name) && index > 0)
    mooseError("Postprocessor requested with index ",
               index,
               " when only a single postprocessor is coupled.");

  // Return the default if the Postprocessor does not exist and a default does, otherwise
  // continue as usual
  if (!hasPostprocessor(name, index) && _ppi_params.hasDefaultPostprocessorValue(name, index))
    return _ppi_params.getDefaultPostprocessorValue(name, false, index);
  else
  {
    if (singlePostprocessor(name))
      return getPostprocessorValueByNameHelper(_ppi_params.get<PostprocessorName>(name), t_index);

    // check size of parameter array here
    if (index >= _ppi_params.get<std::vector<PostprocessorName>>(name).size())
      mooseError("Postprocessor requested with index ",
                 index,
                 " but parameter ",
                 name,
                 " has only ",
                 _ppi_params.get<std::vector<PostprocessorName>>(name).size(),
                 " entries.");

    return getPostprocessorValueByNameHelper(
        _ppi_params.get<std::vector<PostprocessorName>>(name)[index], t_index);
  }
}

const PostprocessorValue &
PostprocessorInterface::getPostprocessorValueByNameHelper(const PostprocessorName & name,
                                                          std::size_t t_index) const
{
  ReporterName r_name(name, "value");
  return _pi_feproblem.getReporterDataInternal().getReporterValue<PostprocessorValue>(
      r_name, name, REPORTER_MODE_ROOT, t_index);
}
