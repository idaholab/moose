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
#include "RayTracingNemesis.h"
#include "libmesh/restore_warnings.h"

// Local includes
#include "RayTracingStudy.h"

// libMesh includes
#include "libmesh/nemesis_io.h"

registerMooseObject("RayTracingApp", RayTracingNemesis);

InputParameters
RayTracingNemesis::validParams()
{
  auto params = RayTracingMeshOutput::validParams();
  params.addClassDescription("Outputs ray segments and data as segments using the Nemesis format.");
  return params;
}

RayTracingNemesis::RayTracingNemesis(const InputParameters & params) : RayTracingMeshOutput(params)
{
}

void
RayTracingNemesis::outputMesh()
{
  TIME_SECTION("outputMesh", 3, "Outputting Nemesis RayTracing Mesh");

  // We write a new file every time for nemesis because the mesh changes
  _file_num++;

  // Build the nemesis IO object
  Nemesis_IO nemesis_io(*_segment_mesh);

  // With nodal data, we need to output these variables in write_timestep
  if (_output_data_nodal)
    nemesis_io.set_output_variables(_study.rayDataNames());
  // Otherwise, there's no variables to write in write_timestep
  else
    nemesis_io.set_output_variables(std::vector<std::string>());
  // Write the timestep, which is the mesh + nodal vars (if any)
  nemesis_io.write_timestep(filename(), *_es, 1, time() + _app.getGlobalTimeOffset());

  // This will default to write_element_data getting all available elemental vars
  nemesis_io.set_output_variables(std::vector<std::string>());
  // Write the elemental variables, which are the variables with the constant Ray field data
  nemesis_io.write_element_data(*_es);
}
