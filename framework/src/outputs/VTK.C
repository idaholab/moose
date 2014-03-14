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

#include "VTK.h"

template<>
InputParameters validParams<VTKOutputter>()
{
  InputParameters params = validParams<OversampleOutputter>();

  // Supress un-available parameters
  params.suppressParameter<bool>("output_scalar_variables");
  params.suppressParameter<bool>("output_postprocessors");
  params.suppressParameter<bool>("scalar_as_nodal");
  params.suppressParameter<bool>("sequence");

  // Set default padding to 3
  params.set<unsigned int>("padding") = 3;

  // Add binary toggle
  params.addParam<bool>("binary", false, "Set VTK files to output in binary format");
  params.addParamNamesToGroup("binary", "Advanced");

  return params;
}

VTKOutputter::VTKOutputter(const std::string & name, InputParameters & parameters) :
    OversampleOutputter(name, parameters),
    _vtk_io_ptr(NULL),
    _binary(getParam<bool>("binary"))
{
  // VTK files must be written in sequence
  sequence(true);
}

VTKOutputter::~VTKOutputter()
{
  delete _vtk_io_ptr;
}

void
VTKOutputter::outputSetup()
{
  // The libMesh::ExodusII_IO will fail when it is closed if the object is created but
  // nothing is written to the file. This checks that at least something will be written.
  if (!hasOutput())
    mooseError("The current settings result in nothing being output to the VTKOutputter file.");

  // Delete existing VTKIO objects
  if (_vtk_io_ptr != NULL)
    delete _vtk_io_ptr;

#ifdef LIBMESH_HAVE_VTK
  // Create the new VTKOutputter object and set compression
  _vtk_io_ptr = new VTKIO(_es_ptr->get_mesh());
  _vtk_io_ptr->set_compression(_binary);
#else
  mooseError("libMesh not configured with VTK");
#endif
}

void
VTKOutputter::output()
{
#ifdef LIBMESH_HAVE_VTK
  // Write the data
  _vtk_io_ptr->write_equation_systems(filename(), *_es_ptr);
#else
  mooseError("libMesh not configured with VTK");
#endif
}

std::string
VTKOutputter::filename()
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
         << _t_step
         << ".vtk";

  // Return the filename
  return output.str();
}

void
VTKOutputter::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not support for VTK output");
}

void
VTKOutputter::outputElementalVariables()
{
  mooseError("Individual output of elemental variables is not support for VTK output");
}

void
VTKOutputter::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not support for VTK output");
}

void
VTKOutputter::outputScalarVariables()
{
  mooseError("Individual output of scalars is not support for VTK output");
}
