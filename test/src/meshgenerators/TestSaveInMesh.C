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

registerMooseObject("MooseTestApp", TestSaveInMesh);

InputParameters
TestSaveInMesh::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<bool>(
      "nemesis", false, "Whether or not to output the mesh file in distributed form");

  return params;
}

TestSaveInMesh::TestSaveInMesh(const InputParameters & parameters) : GeneralUserObject(parameters)
{
  auto & mesh_generator_system = _app.getMeshGeneratorSystem();
  auto saved_mesh_names = mesh_generator_system.getSavedMeshesNames();

  if (saved_mesh_names.size() != 0)
  {
    for (auto & name : saved_mesh_names)
    {
      if (name != mesh_generator_system.mainMeshGeneratorName())
      {
        auto mesh = mesh_generator_system.getSavedMeshes(name);

        if (!getParam<bool>("nemesis"))
        {
          ExodusII_IO exio(*mesh);

          if (mesh->mesh_dimension() == 1)
            exio.write_as_dimension(3);

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
