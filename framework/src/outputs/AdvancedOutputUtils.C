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

// MOOSE includes
#include "AdvancedOutputUtils.h"
#include "Output.h"
#include "InputParameters.h"

// Constructor of OutputOnWarehouse; initializes the MultiMooseEnums for all available output types
OutputOnWarehouse::OutputOnWarehouse(const MultiMooseEnum & execute_on,
                                     const InputParameters & parameters)
  : OutputMapWrapper<MultiMooseEnum>()
{
  // Initialize each of the 'execute_on' settings for the various types of outputs
  if (parameters.have_parameter<MultiMooseEnum>("execute_nodal_on"))
    _map.insert(std::make_pair("nodal", execute_on));

  if (parameters.have_parameter<MultiMooseEnum>("execute_elemental_on"))
    _map.insert(std::make_pair("elemental", execute_on));

  if (parameters.have_parameter<MultiMooseEnum>("execute_scalars_on"))
    _map.insert(std::make_pair("scalars", execute_on));

  if (parameters.have_parameter<MultiMooseEnum>("execute_postprocessors_on"))
    _map.insert(std::make_pair("postprocessors", execute_on));

  if (parameters.have_parameter<MultiMooseEnum>("execute_vector_postprocessors_on"))
    _map.insert(std::make_pair("vector_postprocessors", execute_on));

  if (parameters.have_parameter<MultiMooseEnum>("execute_input_on"))
    _map.insert(std::make_pair("input", MooseUtils::createExecuteOnEnum()));

  if (parameters.have_parameter<MultiMooseEnum>("execute_system_information_on"))
    _map.insert(
        std::make_pair("system_information", MooseUtils::createExecuteOnEnum(1, EXEC_INITIAL)));
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
}
