//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMeshGeneratorMesh.h"
#include "MFEMMeshCarrier.h"
#include "libmesh/mesh_generation.h"

registerMooseObject("MooseApp", MFEMMeshGeneratorMesh);

InputParameters
MFEMMeshGeneratorMesh::validParams()
{
  InputParameters params = MFEMMesh::validParams();

  // Mark this as a mesh-generator-based mesh so SetupMeshAction routes the
  // pipeline output to us via setMeshBase() before buildMesh() is called.
  params.set<bool>("_mesh_generator_mesh") = true;

  params.addClassDescription(
      "Mesh type for building an mfem::ParMesh from a chain of MFEMMeshGenerator objects.");

  return params;
}

MFEMMeshGeneratorMesh::MFEMMeshGeneratorMesh(const InputParameters & parameters)
  : MFEMMesh(parameters)
{
}

std::unique_ptr<MooseMesh>
MFEMMeshGeneratorMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}

mfem::Mesh
MFEMMeshGeneratorMesh::buildSerialMFEMMesh()
{
  if (!hasMeshBase())
    mooseError("MFEMMeshGeneratorMesh: the mesh base has not been set. "
               "Ensure MFEM mesh generators are present in the input file.");

  auto * carrier = dynamic_cast<MFEMMeshCarrier *>(&getMesh());
  if (!carrier)
    mooseError("MFEMMeshGeneratorMesh requires the final mesh generator to produce an MFEM mesh. "
               "If you are using libMesh generators, use MeshGeneratorMesh instead.");

  return carrier->releaseMFEMMesh();
}

#endif
