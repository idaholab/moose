//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMeshGenerator.h"
#include "MFEMMeshCarrier.h"
#include "libmesh/mesh_generation.h"

InputParameters
MFEMMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  // Private flag checked by SetupMeshAction to auto-select MFEMMeshGeneratorMesh
  // when no [Mesh] type is specified by the user.
  params.addPrivateParam<bool>("_mfem_mesh_generator", true);
  return params;
}

MFEMMeshGenerator::MFEMMeshGenerator(const InputParameters & parameters) : MeshGenerator(parameters)
{
}

std::unique_ptr<MeshBase>
MFEMMeshGenerator::generate()
{
  auto carrier = std::make_unique<MFEMMeshCarrier>(comm());
  carrier->setMFEMMesh(generateMFEMMesh());
  // Give the libMesh side a valid dummy so Assembly can read a sane mesh dimension
  // regardless of which MooseMesh type consumes the pipeline output.
  MeshTools::Generation::build_point(*carrier);
  return carrier;
}

mfem::Mesh
MFEMMeshGenerator::getMFEMInputMesh(std::unique_ptr<MeshBase> & mesh_ref)
{
  // Move ownership out, which also nulls mesh_ref for the pipeline's release validation.
  auto owned = std::move(mesh_ref);
  auto * carrier = dynamic_cast<MFEMMeshCarrier *>(owned.get());
  if (!carrier)
    mooseError("An input mesh is not an MFEM mesh. MFEM mesh generators can only accept "
               "inputs from other MFEM mesh generators.");
  return carrier->releaseMFEMMesh();
}

#endif
