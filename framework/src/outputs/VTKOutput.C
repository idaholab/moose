//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VTKOutput.h"

#include "libmesh/vtk_io.h"
#include "libmesh/equation_systems.h"

registerMooseObjectAliased("MooseApp", VTKOutput, "VTK");

InputParameters
VTKOutput::validParams()
{
  InputParameters params = OversampleOutput::validParams();
  params.addClassDescription("Output data using the Visualization Toolkit (VTK).");

  // Set default padding to 3
  params.set<unsigned int>("padding") = 3;

  // Add binary toggle
  params.addParam<bool>("binary", false, "Set VTK files to output in binary format");
  params.addParamNamesToGroup("binary", "Advanced");

  return params;
}

VTKOutput::VTKOutput(const InputParameters & parameters)
  : OversampleOutput(parameters), _binary(getParam<bool>("binary"))
{
#ifndef LIBMESH_HAVE_VTK
  mooseError("VTK output was requested, but libMesh was not configured with VTK. To fix this, you "
             "must reconfigure libMesh to use VTK.");
#endif
}

void
VTKOutput::output()
{
#ifdef LIBMESH_HAVE_VTK
  /// Create VTKIO object
  libMesh::VTKIO vtk(_es_ptr->get_mesh());

  // Set the comppression
  vtk.set_compression(_binary);

  // Write the data
  vtk.write_equation_systems(filename(), *_es_ptr);
  _file_num++;
#endif
}

std::string
VTKOutput::filename()
{
  // Append the .e extension on the base file name
  std::ostringstream output;
  output << _file_base;

  // In parallel, add the _00x.pvtu extension.
  // In serial, add the _00x.pvtu extension anyway - libMesh outputs
  // PVTU format regardless of what file name we give it.
  const std::string ext = ".pvtu";
  output << "_" << std::setw(_padding) << std::setfill('0') << std::right << _file_num << ext;

  // Return the filename
  return output.str();
}
