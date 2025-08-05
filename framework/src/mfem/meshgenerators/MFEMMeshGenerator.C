//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MFEMMeshGenerator.h"
#include "MFEMMesh.h"
#include "libmesh/mesh_generation.h"
#include "CastUniquePointer.h"

registerMooseObject("MooseApp", MFEMMeshGenerator);

InputParameters
MFEMMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshFileName>("file", "The filename to read.");
  params.addClassDescription("Class to read in and store an mfem::ParMesh from file.");

  return params;
}

MFEMMeshGenerator::MFEMMeshGenerator(const InputParameters & parameters) : MeshGenerator(parameters) {}

std::unique_ptr<MeshBase>
MFEMMeshGenerator::generate()
{
  InputParameters params = _app.getFactory().getValidParams("MFEMMesh");
  params.set<MeshFileName>("file")=getParam<MeshFileName>("file");
  std::unique_ptr<MFEMMesh> mesh = _app.getFactory().createUnique<MFEMMesh>("MFEMMesh", "mfem_mesh", params);
  mesh->setMeshBase(buildMeshBaseObject());
  mesh->buildMesh();
  return dynamic_pointer_cast<MeshBase>(mesh);
}

#endif
