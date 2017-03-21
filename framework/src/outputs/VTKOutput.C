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

#include "VTKOutput.h"

// libMesh includes
#include "libmesh/vtk_io.h"
#include "libmesh/equation_systems.h"

template <>
InputParameters
validParams<VTKOutput>()
{
  InputParameters params = validParams<BasicOutput<OversampleOutput>>();

  // Set default padding to 3
  params.set<unsigned int>("padding") = 3;

  // Add binary toggle
  params.addParam<bool>("binary", false, "Set VTK files to output in binary format");
  params.addParamNamesToGroup("binary", "Advanced");

  return params;
}

VTKOutput::VTKOutput(const InputParameters & parameters)
  : BasicOutput<OversampleOutput>(parameters), _binary(getParam<bool>("binary"))
{
}

void
VTKOutput::output(const ExecFlagType & /*type*/)
{
#ifdef LIBMESH_HAVE_VTK

  /// Create VTKIO object
  VTKIO vtk(_es_ptr->get_mesh());

  // Set the comppression
  vtk.set_compression(_binary);

  // Write the data
  vtk.write_equation_systems(filename(), *_es_ptr);
  _file_num++;

#else
  mooseError("libMesh not configured with VTK");
#endif
}

std::string
VTKOutput::filename()
{
  // Append the .e extension on the base file name
  std::ostringstream output;
  output << _file_base;

  // In serial, add the _00x.vtk extension.
  // In parallel, add the _00x.pvtu extension.
  std::string ext = (n_processors() == 1) ? ".vtk" : ".pvtu";
  output << "_" << std::setw(_padding) << std::setfill('0') << std::right << _file_num << ext;

  // Return the filename
  return output.str();
}
