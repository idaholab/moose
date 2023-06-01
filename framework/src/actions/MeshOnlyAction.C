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
#include "AddOutputAction.h"

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
  std::string mesh_file = _app.parameters().get<std::string>("mesh_only");
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

    // Check if extra element integers should be outputted to Exodus file
    unsigned int n_eeid = output_mesh.n_elem_integers();

    // Iterate through all actions and see if `Outputs/output_extra_element_ids = true` in input
    // file
    const auto & output_actions = _app.actionWarehouse().getActionListByName("add_output");
    // Truth of whether to output extra element ids is initially determined by whether
    // there are extra element ids defined on the mesh
    bool output_extra_ids = (n_eeid > 0);
    bool restrict_element_id_names = false;
    std::vector<std::string> element_id_names;
    for (const auto & act : output_actions)
    {
      // Extract the Output action
      AddOutputAction * action = dynamic_cast<AddOutputAction *>(act);
      if (!action)
        continue;

      InputParameters & params = action->getObjectParams();
      if (params.isParamSetByUser("output_extra_element_ids"))
      {
        // User has set output_extra_element_ids, truth of output_extra_ids determined by value of
        // parameter
        output_extra_ids = params.get<bool>("output_extra_element_ids");
        if (output_extra_ids)
        {
          // `Outputs/extra_element_ids_to_output` sets a subset of extra element ids that should
          // be outputted to Exodus
          restrict_element_id_names = params.isParamValid("extra_element_ids_to_output");
          if (restrict_element_id_names)
          {
            element_id_names = params.get<std::vector<std::string>>("extra_element_ids_to_output");
            // Check which element id names actually are defined on the mesh, remove from
            // element_id_names if they don't belong
            for (auto it = element_id_names.begin(); it != element_id_names.end();)
            {
              // Erase contents of iterator and return iterator if element integer does not exist
              if (!output_mesh.has_elem_integer(*it))
              {
                it = element_id_names.erase(it);
                mooseWarning("Extra element id ",
                             *it,
                             " defined in Outputs/extra_element_ids_to_output "
                             "is not defined on the mesh and will be ignored.");
              }
              // Increment iterator if element integer exists
              else
                ++it;
            }
          }
        }
        break;
      }
    }

    if (output_extra_ids)
    {
      // Retrieve extra element id names and associated data
      const auto n_elem = output_mesh.n_elem();
      std::vector<std::string> eeid_vars;
      const auto n_eeid_to_output = restrict_element_id_names ? element_id_names.size() : n_eeid;
      std::vector<Number> eeid_soln(n_elem * n_eeid_to_output);
      unsigned int soln_index = 0;
      for (unsigned int i = 0; i < n_eeid; i++)
      {
        const auto eeid_name = output_mesh.get_elem_integer_name(i);
        // If `Outputs/extra_element_ids_to_output` is not set, output all ids to Exodus
        // Otherwise only output if the extra id name is contained within
        // `Outputs/extra_element_ids_to_output`
        if (!restrict_element_id_names ||
            (std::find(element_id_names.begin(), element_id_names.end(), eeid_name) !=
             element_id_names.end()))
        {
          eeid_vars.push_back(output_mesh.get_elem_integer_name(i));
          for (const auto & elem : output_mesh.element_ptr_range())
          {
            eeid_soln[soln_index] = (int)elem->get_extra_integer(i);
            ++soln_index;
          }
        }
      }

      // Check size of output variables just in case none of the variables in
      // `Outpus/extra_element_ids_to_output` are specified on the actual mesh
      if (eeid_vars.size() > 0)
      {
        // Invoke ExodusII_IO_Helper to output extra element ids to Exodus file
        auto & exio_helper = exio.get_exio_helper();

        // Output empty timestep to Exodus file
        int empty_timestep = 1;
        Real default_time = 1.0;
        exio_helper.write_timestep(empty_timestep, default_time);

        // Write extra element id data to Exodus file
        std::vector<std::set<subdomain_id_type>> vars_active_subdomains;
        vars_active_subdomains.resize(n_eeid_to_output);
        exio_helper.initialize_element_variables(eeid_vars, vars_active_subdomains);
        exio_helper.write_element_values(
            output_mesh, eeid_soln, empty_timestep, vars_active_subdomains);
      }
      else
        mooseWarning(
            "Outputs/output_extra_element_ids is set to true but no extra element ids are being "
            "outputted. Please check extra element ids are properly defined on the mesh and any "
            "variables specified in Outputs/extra_element_ids_to_output are spelled correctly.");
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
