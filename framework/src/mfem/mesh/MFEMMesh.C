//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMesh.h"
#include "MooseApp.h"
#include "libmesh/mesh_generation.h"

#include <fstream>

InputParameters
MFEMMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addParam<unsigned int>(
      "serial_refine",
      0,
      "Number of serial refinements to perform on the mesh. Equivalent to uniform_refine.");
  params.addParam<unsigned int>(
      "uniform_refine",
      0,
      "Number of serial refinements to perform on the mesh. Equivalent to serial_refine.");
  params.addParam<unsigned int>(
      "parallel_refine", 0, "Number of parallel refinements to perform after partitioning.");
  params.addParam<bool>("nonconforming",
                        false,
                        "Ensures the mesh is non-conforming: necessary for refining quad/hex "
                        "meshes and load (re)balancing.");
  params.addParam<bool>("reorder_mesh",
                        false,
                        "Reorder elements with Hilbert space-filling curve ordering before "
                        "partitioning, to improve dynamic load balancing.");
  params.addParam<std::string>("displacement",
                               "Optional variable name to use for mesh displacement.");
  return params;
}

MFEMMesh::MFEMMesh(const InputParameters & parameters) : MooseMesh(parameters) {}

void
MFEMMesh::init()
{
  // MooseMesh::init() handles the libMesh dummy mesh:
  //   - recovery:    reads the libMesh mesh back from its checkpoint file
  //   - normal run:  calls buildMesh(), which for MFEMMesh builds both the
  //                  dummy libMesh mesh and the MFEM ParMesh
  MooseMesh::init();

  if (_app.isRecovering() && allowRecovery() && _app.isUltimateMaster())
  {
    // MooseMesh::init() already restored the libMesh dummy mesh from its checkpoint.
    // Now restore the MFEM parallel mesh from its own checkpoint file.
    const auto checkpoint_file = _app.getRestartRecoverFileBase() + _app.checkpointSuffix() +
                                 ".mfem.mesh." + std::to_string(this->processor_id());
    std::ifstream input(checkpoint_file);
    if (!input)
      mooseError("Unable to open MFEM recovery mesh file '", checkpoint_file, "'.");

    _mfem_par_mesh = std::make_shared<mfem::ParMesh>(this->comm().get(), input);

    if (isParamSetByUser("displacement"))
      _mesh_displacement_variable.emplace(getParam<std::string>("displacement"));
  }
}

void
MFEMMesh::buildMesh()
{
  TIME_SECTION("buildMesh", 2, "Building MFEM Mesh");

  mfem::Mesh ser_mesh = buildSerialMFEMMesh();

  if (isParamSetByUser("serial_refine") && isParamSetByUser("uniform_refine"))
    paramError("serial_refine",
               "Cannot set both serial_refine and uniform_refine at the same time.");

  uniformRefinement(ser_mesh,
                    isParamSetByUser("serial_refine") ? getParam<unsigned int>("serial_refine")
                                                      : getParam<unsigned int>("uniform_refine"));

  if (getParam<bool>("reorder_mesh"))
  {
    mfem::Array<int> ordering;
    ser_mesh.GetHilbertElementOrdering(ordering);
    ser_mesh.ReorderElements(ordering);
  }

  if (getParam<bool>("nonconforming"))
    ser_mesh.EnsureNCMesh(true);

  _mfem_par_mesh = std::make_shared<mfem::ParMesh>(this->comm().get(), ser_mesh);
  uniformRefinement(*_mfem_par_mesh, getParam<unsigned int>("parallel_refine"));

  if (isParamSetByUser("displacement"))
    _mesh_displacement_variable.emplace(getParam<std::string>("displacement"));

  // Build a dummy MOOSE mesh to enable this class to work with other MOOSE classes.
  buildDummyMooseMesh();
}

std::vector<std::filesystem::path>
MFEMMesh::writeRecoveryFiles(const std::filesystem::path & file_base)
{
  MooseMesh::writeRecoveryFiles(file_base);

  mooseAssert(_mfem_par_mesh, "MFEM parallel mesh is not initialized");

  const auto checkpoint_file =
      file_base.string() + ".mfem.mesh." + std::to_string(this->processor_id());
  std::ofstream output(checkpoint_file);
  if (!output)
    mooseError("Unable to open MFEM recovery mesh file '", checkpoint_file, "' for writing.");

  _mfem_par_mesh->ParPrint(output);
  return {checkpoint_file};
}

void
MFEMMesh::displace(mfem::GridFunction const & displacement)
{
  _mfem_par_mesh->EnsureNodes();
  *_mfem_par_mesh->GetNodes() += displacement;
}

void
MFEMMesh::buildDummyMooseMesh()
{
  // The libMesh placeholder is always replicated, independently of the distributed MFEM mesh.
  setParallelType(ParallelType::REPLICATED);
  auto & dummy = static_cast<UnstructuredMesh &>(getMesh());
  MeshTools::Generation::build_point(dummy);
  if (dimension() >= 2)
    MeshTools::Generation::build_square(dummy, 1, 1, 0., 1., 0., 1., ElemType::QUAD9);
}

void
MFEMMesh::uniformRefinement(mfem::Mesh & mesh, const unsigned int nref) const
{
  for (unsigned int i = 0; i < nref; ++i)
    mesh.UniformRefinement();
}

#endif
