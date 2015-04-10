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

// Constructor of OutputOnWarehouse; initializes the MultiMooseEnums for all available output types
OutputOnWarehouse::OutputOnWarehouse(const MultiMooseEnum & output_on, const InputParameters & params) : OutputMapWrapper<MultiMooseEnum>()
{
  // Initialize each of the output_on settings for the various types of outputs
  if (params.have_parameter<MultiMooseEnum>("output_nodal_on"))
    _map.insert(std::make_pair("nodal", output_on));

  if (params.have_parameter<MultiMooseEnum>("output_elemental_on"))
    _map.insert(std::make_pair("elemental", output_on));

  if (params.have_parameter<MultiMooseEnum>("output_scalars_on"))
    _map.insert(std::make_pair("scalars", output_on));

  if (params.have_parameter<MultiMooseEnum>("output_postprocessors_on"))
    _map.insert(std::make_pair("postprocessors", output_on));

  if (params.have_parameter<MultiMooseEnum>("output_vector_postprocessors_on"))
    _map.insert(std::make_pair("vector_postprocessors", output_on));

  if (params.have_parameter<MultiMooseEnum>("output_input_on"))
    _map.insert(std::make_pair("input", Output::getExecuteOptions()));

  if (params.have_parameter<MultiMooseEnum>("output_system_information_on"))
    _map.insert(std::make_pair("system_information", Output::getExecuteOptions("initial")));
}

// Constructor of OutputDataWarehouse; initializes the OutputData structures for 'variable' based output types
OutputDataWarehouse::OutputDataWarehouse() : OutputMapWrapper<OutputData>()
{
  _map["nodal"] = OutputData();
  _map["elemental"] = OutputData();
  _map["scalars"] = OutputData();
  _map["postprocessors"] = OutputData();
  _map["vector_postprocessors"] = OutputData();
}
