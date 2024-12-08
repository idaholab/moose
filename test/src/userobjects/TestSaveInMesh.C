//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSaveInMesh.h"

#include "Exodus.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/checkpoint_io.h"
#include "libmesh/nemesis_io.h"

using namespace libMesh;

registerMooseObject("MooseTestApp", TestSaveInMesh);

InputParameters
TestSaveInMesh::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<std::string>(
      "find_mesh",
      std::string(),
      "Test whether or not throw out an error when the saved mesh is not found");
  params.addParam<std::string>(
      "mesh_unique",
      std::string(),
      "Test whether or not throw out an error when the saved mesh mesh has already been retrieved");
  return params;
}

TestSaveInMesh::TestSaveInMesh(const InputParameters & parameters) : GeneralUserObject(parameters)
{
  auto & mesh_generator_system = _app.getMeshGeneratorSystem();
  auto saved_mesh_names = mesh_generator_system.getSavedMeshNames();

  if (saved_mesh_names.size() != 0)
  {
    for (auto & name : saved_mesh_names)
    {
      if (name != mesh_generator_system.mainMeshGeneratorName())
      {
        auto mesh = mesh_generator_system.getSavedMesh(name);

        ExodusII_IO exio(*mesh);

        if (mesh->mesh_dimension() == 1)
          exio.write_as_dimension(3);

        exio.set_hdf5_writing(false);

        exio.write(name + "_in.e");
      }
    }

    auto find_mesh_name = getParam<std::string>("find_mesh");
    if (!find_mesh_name.empty())
    {
      auto find_mesh = mesh_generator_system.getSavedMesh(find_mesh_name);
    }

    auto mesh_unique_test = getParam<std::string>("mesh_unique");
    if (!mesh_unique_test.empty())
    {
      auto test_mesh1 = mesh_generator_system.getSavedMesh(mesh_unique_test);
      auto test_mesh2 = mesh_generator_system.getSavedMesh(mesh_unique_test);
    }
  }
}
