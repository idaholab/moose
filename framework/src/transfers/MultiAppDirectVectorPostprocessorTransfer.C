//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppDirectVectorPostprocessorTransfer.h"

// MOOSE includes
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"

registerMooseObject("MooseApp", MultiAppDirectVectorPostprocessorTransfer);

InputParameters
MultiAppDirectVectorPostprocessorTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription(
      "Copy vector(s) of a VectorPostprocessor to/from the main and sub-applications(s).");
  params.addRequiredParam<VectorPostprocessorName>(
      "to_vector_postprocessor", "The name of the VectorPostprocessor that is accepting data.");
  params.addRequiredParam<VectorPostprocessorName>(
      "from_vector_postprocessor", "The name of the VectorPostprocessor that is producing data.");
  params.addParam<unsigned int>(
      "subapp_index",
      std::numeric_limits<unsigned int>::max(),
      "The MultiApp object sub-application index to use when transferring to/from the "
      "sub-application. If unset and transferring to the sub-applications then all "
      "sub-applications will receive data. The value must be set when transferring from a "
      "sub-application.");
  params.addRequiredParam<std::vector<std::string>>(
      "vector_names", "Named vector(s) to transfer from/to in VectorPostprocessor.");
  return params;
}

MultiAppDirectVectorPostprocessorTransfer::MultiAppDirectVectorPostprocessorTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _to_vpp_name(getParam<VectorPostprocessorName>("to_vector_postprocessor")),
    _from_vpp_name(getParam<VectorPostprocessorName>("from_vector_postprocessor")),
    _vector_names(getParam<std::vector<std::string>>("vector_names")),
    _subapp_index(getParam<unsigned int>("subapp_index"))
{
  if (_directions.size() != 1)
    paramError("direction", "This transfer is only unidirectional");
}

void
MultiAppDirectVectorPostprocessorTransfer::executeToMultiapp()
{
  for (const auto & vec_name : _vector_names)
  {
    const VectorPostprocessorValue & from_vpp =
        _multi_app->problemBase().getVectorPostprocessorValueByName(_from_vpp_name, vec_name);

    if (_subapp_index == std::numeric_limits<unsigned int>::max())
    {
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
        if (_multi_app->hasLocalApp(i))
          _multi_app->appProblemBase(i).setVectorPostprocessorValueByName(
              _to_vpp_name, vec_name, from_vpp);
    }

    else if (_multi_app->hasLocalApp(_subapp_index))
      _multi_app->appProblemBase(_subapp_index)
          .setVectorPostprocessorValueByName(_to_vpp_name, vec_name, from_vpp);

    else if (_subapp_index >= _multi_app->numGlobalApps())
      paramError("subapp_index",
                 "The supplied sub-application index (",
                 _subapp_index,
                 ") is greater than the number of sub-applications.");
  }
}

void
MultiAppDirectVectorPostprocessorTransfer::executeFromMultiapp()
{
  if (_subapp_index >= _multi_app->numGlobalApps())
    paramError("subapp_index",
               "The supplied sub-application index (",
               _subapp_index,
               ") is greater than the number of sub-applications.");

  for (const auto & vec_name : _vector_names)
  {
    const VectorPostprocessorValue & from_vpp =
        _multi_app->appProblemBase(_subapp_index)
            .getVectorPostprocessorValueByName(_from_vpp_name, vec_name);

    _multi_app->problemBase().setVectorPostprocessorValueByName(_to_vpp_name, vec_name, from_vpp);
  }
}

void
MultiAppDirectVectorPostprocessorTransfer::execute()
{
  _console << "Beginning VectorPostprocessorTransfer" << name() << std::endl;
  if (_current_direction == FROM_MULTIAPP)
    executeFromMultiapp();
  else
    executeToMultiapp();
  _console << "Finished VectorPostprocessorTransfer" << name() << std::endl;
}
