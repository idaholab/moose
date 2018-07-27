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
#include "libmesh/exodusII_io.h"

registerMooseAction("MooseApp", MeshOnlyAction, "mesh_only");

template <>
InputParameters
validParams<MeshOnlyAction>()
{
  return validParams<Action>();
}

MeshOnlyAction::MeshOnlyAction(InputParameters params) : Action(params) {}

void
MeshOnlyAction::act()
{
  auto mesh_file = _app.parameters().get<std::string>("mesh_only");
  auto & mesh_ptr = _app.actionWarehouse().mesh();

  // Print information about the mesh
  mesh_ptr->getMesh().print_info();

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
    mesh_file = _app.parser().getFileName();
    size_t pos = mesh_file.find_last_of('.');

    // Default to writing out an ExodusII mesh base on the input filename.
    mesh_file = mesh_file.substr(0, pos) + "_in.e";
  }

  // If we're writing an Exodus file, write the Mesh using its logical
  // element dimension rather than the spatial dimension, unless it's
  // a 1D Mesh.  One reason to prefer this approach is that sidesets
  // are displayed incorrectly for 2D triangular elements in both
  // Paraview and Cubit if num_dim==3 in the Exodus file. We do the
  // same thing in MOOSE's Exodus Output object, so we are mimicking
  // that behavior here.
  if (mesh_file.find(".e") + 2 == mesh_file.size())
  {
    ExodusII_IO exio(mesh_ptr->getMesh());
    if (mesh_ptr->getMesh().mesh_dimension() != 1)
      exio.use_mesh_dimension_instead_of_spatial_dimension(true);

    exio.write(mesh_file);
  }
  else
  {
    // Just write the file using the name requested by the user.
    mesh_ptr->getMesh().write(mesh_file);
  }
}
