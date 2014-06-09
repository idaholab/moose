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

template<>
InputParameters validParams<VTKOutput>()
{
  InputParameters params = validParams<OversampleOutput>();

  // Supress un-available parameters
  params.suppressParameter<bool>("output_scalar_variables");
  params.suppressParameter<bool>("output_postprocessors");
  params.suppressParameter<bool>("output_vector_postprocessors");
  params.suppressParameter<bool>("scalar_as_nodal");
  params.suppressParameter<bool>("sequence");
  params.suppressParameter<bool>("output_input");

  // Set default padding to 3
  params.set<unsigned int>("padding") = 3;

  // Add binary toggle
  params.addParam<bool>("binary", false, "Set VTKOutput files to output in binary format");
  params.addParamNamesToGroup("binary", "Advanced");

  return params;
}

VTKOutput::VTKOutput(const std::string & name, InputParameters & parameters) :
    OversampleOutput(name, parameters),
    _vtk_io_ptr(NULL),
    _binary(getParam<bool>("binary"))
{
  // VTKOutput files must be written in sequence
  sequence(true);
}

VTKOutput::~VTKOutput()
{
  delete _vtk_io_ptr;
}

void
VTKOutput::outputSetup()
{
  // The libMesh::VTK_IO will fail when it is closed if the object is created but
  // nothing is written to the file. This checks that at least something will be written.
  if (!hasOutput())
    mooseError("The current settings result in nothing being output to the VTKOutput file.");

  // Delete existing VTKOutputIO objects
  if (_vtk_io_ptr != NULL)
    delete _vtk_io_ptr;

#ifdef LIBMESH_HAVE_VTK
  // Create the new VTKOutput object and set compression
  _vtk_io_ptr = new VTKIO(_es_ptr->get_mesh());
  _vtk_io_ptr->set_compression(_binary);
#else
  mooseError("libMesh not configured with VTKOutput");
#endif
}

void
VTKOutput::output()
{
#ifdef LIBMESH_HAVE_VTK
  // Write the data
  _vtk_io_ptr->write_equation_systems(filename(), *_es_ptr);
  _file_num++;
#else
  mooseError("libMesh not configured with VTKOutput");
#endif
}

std::string
VTKOutput::filename()
{
  // Append the .e extension on the base file name
  std::ostringstream output;
  output << _file_base;

  // Add the _00x.vtk extension to the file
  output << "_"
         << std::setw(_padding)
         << std::setprecision(0)
         << std::setfill('0')
         << std::right
         << _file_num
         << ".vtk";

  // Return the filename
  return output.str();
}

void
VTKOutput::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not supported for VTKOutput output");
}

void
VTKOutput::outputElementalVariables()
{
  mooseError("Individual output of elemental variables is not supported for VTKOutput output");
}

void
VTKOutput::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not supported for VTKOutput output");
}

void
VTKOutput::outputVectorPostprocessors()
{
  mooseError("Individual output of VectorPostprocessors is not supported for VTKOutput output");
}

void
VTKOutput::outputScalarVariables()
{
  mooseError("Individual output of scalars is not supported for VTKOutput output");
}
