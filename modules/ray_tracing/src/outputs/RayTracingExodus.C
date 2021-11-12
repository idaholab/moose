//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// TODO: remove ignore warnings once std::tuple<> is instantiated as a StandardType
// Using push_parallel_vector_data with std::tuple<> leads to a -Wextra error
// https://github.com/libMesh/TIMPI/issues/52
#include "libmesh/ignore_warnings.h"
#include "RayTracingExodus.h"
#include "libmesh/restore_warnings.h"

// Local includes
#include "RayTracingStudy.h"

registerMooseObject("RayTracingApp", RayTracingExodus);

InputParameters
RayTracingExodus::validParams()
{
  auto params = RayTracingMeshOutput::validParams();
  params.addClassDescription("Outputs ray segments and data as segments using the Exodus format.");
  return params;
}

RayTracingExodus::RayTracingExodus(const InputParameters & params)
  : RayTracingMeshOutput(params), _exodus_num(1)
{
}

void
RayTracingExodus::outputMesh()
{
  TIME_SECTION("outputMesh", 3, "Writing Ray Mesh");

  // Build the Exodus IO object if it hasn't been built yet
  if (!_exodus_io)
    _exodus_io = std::make_unique<ExodusII_IO>(*_segment_mesh);

  // With nodal data, we need to output these variables in write_timestep
  if (_output_data_nodal)
    _exodus_io->set_output_variables(_study.rayDataNames());
  // Otherwise, there's no variables to write in write_timestep
  else
    _exodus_io->set_output_variables(std::vector<std::string>());
  // Write the timestep, which is the mesh + nodal vars (if any)
  _exodus_io->write_timestep(filename(), *_es, _exodus_num++, time() + _app.getGlobalTimeOffset());

  // This will default to write_element_data getting all available elemental vars
  _exodus_io->set_output_variables(std::vector<std::string>());
  // Write the elemental variables, which are the variables with the constant Ray field data
  _exodus_io->write_element_data(*_es);
}
