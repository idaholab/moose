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
#include "libmesh/checkpoint_io.h"
#include <libmesh/nemesis_io.h>

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
  auto mesh_file = _app.parameters().get<std::string>("mesh_only");
  auto saved_mesh_file = mesh_file;
  auto & mesh_ptr = _app.actionWarehouse().mesh();
  auto saved_mesh_ptr = _app.actionWarehouse().savedMesh();
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
    auto saved_mesh_file = mesh_file.substr(0, pos);
  }

  /**
   * If we're writing an Exodus file, write the Mesh using it's effective spatial dimension unless
   * it's a 1D mesh. This is to work around a Paraview bug where 1D meshes are not visualized
   * properly.
   */
  if (mesh_file.find(".e") + 2 == mesh_file.size())
  {
    TIME_SECTION("act", 1, "Writing Exodus");

    ExodusII_IO exio(mesh_ptr->getMesh());

    Exodus::setOutputDimensionInExodusWriter(exio, *mesh_ptr);

    // Default to non-HDF5 output for wider compatibility
    exio.set_hdf5_writing(false);

    exio.write(mesh_file);
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

  auto & mesh_generator_system = _app.getMeshGeneratorSystem();
  auto saved_mesh_names = mesh_generator_system.getSavedMeshesNames();
  if (saved_mesh_names.size() != 0)
  {
    for (auto & name : saved_mesh_names)
    {
      if (name != mesh_generator_system.mainMeshGeneratorName())
      {
        // We now have ownership of this mesh, it will get destructed out of scope
        auto mesh = mesh_generator_system.getSavedMeshes(name);

        if (mesh->is_replicated())
        {
          ExodusII_IO exio(*mesh);

          if (mesh->mesh_dimension() == 1)
            exio.write_as_dimension(3);

          // Default to non-HDF5 output for wider compatibility
          exio.set_hdf5_writing(false);

          exio.write(name + "_in.e");
        }
        else
        {
          Nemesis_IO nemesis_io(*mesh);
          nemesis_io.write(name + "_in.e");
        }
      }
    }
  }
}
