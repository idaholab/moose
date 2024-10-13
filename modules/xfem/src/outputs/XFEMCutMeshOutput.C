//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMCutMeshOutput.h"
#include "MeshCut2DUserObjectBase.h"

registerMooseObject("XFEMApp", XFEMCutMeshOutput);

InputParameters
XFEMCutMeshOutput::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Outputs XFEM MeshCut2DUserObjectBase cutter mesh in Exodus format.");
  params.addRequiredParam<UserObjectName>("xfem_cutter_uo",
                                          "The MeshCut2DUserObject to get cutter mesh from");
  return params;
}

XFEMCutMeshOutput::XFEMCutMeshOutput(const InputParameters & params)
  : FileOutput(params),
    UserObjectInterface(this),
    _cutter_uo(getUserObject<MeshCut2DUserObjectBase>("xfem_cutter_uo"))
{
}

std::string
XFEMCutMeshOutput::filename()
{
  // Append the .e extension to the base file name
  std::ostringstream output;
  output << _file_base << "_XFEMCutMeshOutput.e";

  // Add the _000x extension to the file
  if (_file_num > 1)
    output << "-s" << std::setw(_padding) << std::setprecision(0) << std::setfill('0') << std::right
           << _file_num;

  // Return the filename
  return output.str();
}

void
XFEMCutMeshOutput::output()
{
  // exodus_num is always one because we are always assuming the mesh changes between outputs
  int exodus_num = 1;
  ++_file_num;
  _es = std::make_unique<EquationSystems>(_cutter_uo.getCutterMesh());
  _exodus_io = std::make_unique<libMesh::ExodusII_IO>(_es->get_mesh());
  // Default to non-HDF5 output for wider compatibility
  _exodus_io->set_hdf5_writing(false);
  _exodus_io->write_timestep(
      filename(), *_es, exodus_num, getOutputTime() + _app.getGlobalTimeOffset());

  // Done with these
  // We don't necessarily need to create a new mesh every time, but it's easier than
  // checking if the cutter mesh has changed from last time we built a mesh
  _es->clear();
  _es = nullptr;
}
