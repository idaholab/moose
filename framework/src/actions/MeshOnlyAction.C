//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshOnlyAction.h"

#include "MooseApp.h"
#include "MooseMesh.h"
#include "Exodus.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/exodusII_io_helper.h"
#include "libmesh/checkpoint_io.h"

registerMooseAction("MooseApp", MeshOnlyAction, "mesh_only");

InputParameters
MeshOnlyAction::validParams()
{
  return Action::validParams();
}

MeshOnlyAction::MeshOnlyAction(const InputParameters & params) : Action(params) {}

void
MeshOnlyAction::act()
{
  bool output_eeid_to_exodus = _app.isParamValid("initialize_only");
  std::string mesh_file = output_eeid_to_exodus ? _app.parameters().get<std::string>("initialize_only") : _app.parameters().get<std::string>("mesh_only");
  auto & mesh_ptr = _app.actionWarehouse().mesh();

  // Print information about the mesh
  _console << mesh_ptr->getMesh().get_info(/* verbosity = */ 2) << std::endl;

  bool should_generate = false;
  // If no argument specified or if the argument following --mesh-only starts
  // with a dash, try to build an output filename based on the input mesh filename.
  if (mesh_file.empty() || (mesh_file[0] == '-'))
    should_generate = true;
  // There's something following the --mesh-only flag, let's make an attempt to validate it.
  // If we don't find a '.' or we DO find an equals sign, chances are this is not a file!
  else if ((mesh_file.find('.') == std::string::npos || mesh_file.find('=') != std::string::npos))
  {
    mooseWarning("The --mesh-only option should be followed by a file name. Move it to the end of "
                 "your CLI args or follow it by another \"-\" argument.");
    should_generate = true;
  }

  if (should_generate)
  {
    mesh_file = _app.parser().getPrimaryFileName();
    size_t pos = mesh_file.find_last_of('.');

    // Default to writing out an ExodusII mesh base on the input filename.
    mesh_file = mesh_file.substr(0, pos) + "_in.e";
  }

  /**
   * If we're writing an Exodus file, write the Mesh using it's effective spatial dimension unless
   * it's a 1D mesh. This is to work around a Paraview bug where 1D meshes are not visualized
   * properly.
   */
  if (mesh_file.find(".e") + 2 == mesh_file.size())
  {
    TIME_SECTION("act", 1, "Writing Exodus");

    auto & output_mesh = mesh_ptr->getMesh();
    ExodusII_IO exio(output_mesh);

    Exodus::setOutputDimensionInExodusWriter(exio, *mesh_ptr);

    // Default to non-HDF5 output for wider compatibility
    exio.set_hdf5_writing(false);

    exio.write(mesh_file);

    unsigned int n_eeid = output_mesh.n_elem_integers();
    if (output_eeid_to_exodus && n_eeid > 0)
    {
      // Invoke ExodusII_IO_Helper to output extra element ids to Exodus file
      auto & exio_helper = exio.get_exio_helper();

      // Output empty timestep to Exodus file
      int empty_timestep = 1;
      Real default_time = 1.0;
      exio_helper.write_timestep(empty_timestep, default_time);

      // Retrieve extra element id names and associated data
      std::vector<std::string> eeid_vars;
      std::vector<Number> eeid_soln;
      for (unsigned int i = 0; i < n_eeid; i++)
      {
        eeid_vars.push_back(output_mesh.get_elem_integer_name(i));
        for (auto & elem : output_mesh.element_ptr_range())
          eeid_soln.push_back(elem->get_extra_integer(i));
      }

      // Write extra element id data to Exodus file
      std::vector<std::set<subdomain_id_type>> vars_active_subdomains;
      vars_active_subdomains.resize(n_eeid);
      exio_helper.initialize_element_variables(eeid_vars, vars_active_subdomains);
      exio_helper.write_element_values(output_mesh, eeid_soln, empty_timestep, vars_active_subdomains);
    }
  }
  else if (mesh_file.find(".cpr") + 4 == mesh_file.size())
  {
    TIME_SECTION("act", 1, "Writing Checkpoint");

    CheckpointIO io(mesh_ptr->getMesh(), true);

    io.write(mesh_file);
  }
  else
  {
    // Just write the file using the name requested by the user.
    mesh_ptr->getMesh().write(mesh_file);
  }
}
