//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/mesh_communication.h"

registerMooseObject("MooseApp", FileMeshGenerator);

defineLegacyParams(FileMeshGenerator);

InputParameters
FileMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshFileName>("file", "The filename to read.");
  params.addParam<std::vector<std::string>>(
      "exodus_extra_element_integers",
      "The variable names in the mesh file for loading extra element integers");
  params.addClassDescription("Read a mesh from a file.");
  return params;
}

FileMeshGenerator::FileMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _file_name(getParam<MeshFileName>("file"))
{
}

std::unique_ptr<MeshBase>
FileMeshGenerator::generate()
{
  auto mesh = _mesh->buildMeshBaseObject();

  bool exodus =
      _file_name.rfind(".exd") < _file_name.size() || _file_name.rfind(".e") < _file_name.size();
  bool has_exodus_integers = isParamValid("exodus_extra_element_integers");
  if (exodus && has_exodus_integers)
  {
    if (mesh->processor_id() == 0)
    {
      ExodusII_IO io(*mesh);
      io.set_extra_integer_vars(
          getParam<std::vector<std::string>>("exodus_extra_element_integers"));
      io.read(_file_name);
    }
    MeshCommunication().broadcast(*mesh);
    mesh->prepare_for_use();
  }
  else if (!has_exodus_integers)
    mesh->read(_file_name);
  else
    mooseError("\"exodus_extra_element_integers\" should be given only for Exodus mesh files");

  return dynamic_pointer_cast<MeshBase>(mesh);
}
