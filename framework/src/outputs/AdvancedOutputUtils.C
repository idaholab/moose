//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AdvancedOutputUtils.h"
#include "Output.h"
#include "InputParameters.h"

void
OutputData::reset()
{
  available.clear();
  show.clear();
  hide.clear();
  output.clear();
}

// Constructor of OutputOnWarehouse; initializes the MultiMooseEnums for all available output types
OutputOnWarehouse::OutputOnWarehouse(const ExecFlagEnum & execute_on,
                                     const InputParameters & parameters)
  : OutputMapWrapper<ExecFlagEnum>()
{
  // Initialize each of the 'execute_on' settings for the various types of outputs
  if (parameters.have_parameter<ExecFlagEnum>("execute_nodal_on"))
    _map.insert(std::make_pair("nodal", execute_on));

  if (parameters.have_parameter<ExecFlagEnum>("execute_elemental_on"))
    _map.insert(std::make_pair("elemental", execute_on));

  if (parameters.have_parameter<ExecFlagEnum>("execute_scalars_on"))
    _map.insert(std::make_pair("scalars", execute_on));

  if (parameters.have_parameter<ExecFlagEnum>("execute_postprocessors_on"))
    _map.insert(std::make_pair("postprocessors", execute_on));

  if (parameters.have_parameter<ExecFlagEnum>("execute_vector_postprocessors_on"))
    _map.insert(std::make_pair("vector_postprocessors", execute_on));

  if (parameters.have_parameter<ExecFlagEnum>("execute_reporters_on"))
    _map.insert(std::make_pair("reporters", execute_on));

  if (parameters.have_parameter<ExecFlagEnum>("execute_input_on"))
    _map.insert(std::make_pair("input", MooseUtils::getDefaultExecFlagEnum()));

  if (parameters.have_parameter<ExecFlagEnum>("execute_system_information_on"))
  {
    ExecFlagEnum flags = MooseUtils::getDefaultExecFlagEnum();
    flags = EXEC_INITIAL;
    _map.insert(std::make_pair("system_information", flags));
  }
}

// Constructor of OutputDataWarehouse; initializes the OutputData structures for 'variable' based
// output types
OutputDataWarehouse::OutputDataWarehouse() : OutputMapWrapper<OutputData>(), _has_show_list(false)
{
  _map["nodal"] = OutputData();
  _map["elemental"] = OutputData();
  _map["scalars"] = OutputData();
  _map["postprocessors"] = OutputData();
  _map["vector_postprocessors"] = OutputData();
  _map["reporters"] = OutputData();
}

void
OutputDataWarehouse::reset()
{
  for (auto map_pair : _map)
    map_pair.second.reset();
}
