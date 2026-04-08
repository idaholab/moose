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

namespace
{
using RestartableParMeshState = MFEMMesh::RestartableParMeshState;
}

template <>
void
dataStore(std::ostream & stream, RestartableParMeshState & /*data*/, void * context)
{
  mooseAssert(context, "Missing MFEM mesh context for restartable storage");
  static_cast<MFEMMesh *>(context)->storeRestartableParMesh(stream);
}

template <>
void
dataLoad(std::istream & stream, RestartableParMeshState & /*data*/, void * context)
{
  mooseAssert(context, "Missing MFEM mesh context for restartable loading");
  static_cast<MFEMMesh *>(context)->loadRestartableParMesh(stream);
}

registerMooseObject("MooseApp", MFEMMesh);

struct MFEMMesh::RestartableParMeshState
{
};

InputParameters
MFEMMesh::validParams()
{
  InputParameters params = FileMesh::validParams();
  params.addParam<unsigned int>(
      "serial_refine",
      0,
      "Number of serial refinements to perform on the mesh. Equivalent to uniform_refine.");
  params.addParam<unsigned int>(
      "uniform_refine",
      0,
      "Number of serial refinements to perform on the mesh. Equivalent to serial_refine");
  params.addParam<unsigned int>(
      "parallel_refine", 0, "Number of parallel refinements to perform on the mesh.");
  params.addParam<std::string>("displacement", "Optional variable to use for mesh displacement.");
  params.addParam<bool>("nonconforming",
                        false,
                        "Ensures the mesh is non-conforming: necessary for refining quad/hex "
                        "meshes and load (re)balancing.");
  params.addParam<bool>("reorder_mesh",
                        false,
                        "Determines whether we reorder the mesh to improve dynamic partitioning. "
                        "Only Hilbert sorting is supported at present.");

  params.addClassDescription("Class to read in and store an mfem::ParMesh from file.");

  return params;
}

MFEMMesh::MFEMMesh(const InputParameters & parameters)
  : FileMesh(parameters),
    _restartable_par_mesh_state(
        declareRestartableDataWithContext<RestartableParMeshState>("mfem_par_mesh_state", this))
{
}

MFEMMesh::~MFEMMesh() {}

void
MFEMMesh::init()
{
  if (!_mesh)
    _mesh = buildMeshBaseObject();

  if (_app.isSplitMesh() && _use_distributed_mesh)
    mooseError("You cannot use the mesh splitter capability with DistributedMesh!");

  TIME_SECTION("init", 2);

  if ((_app.isRecovering() || _app.isRestarting() || _app.hasInitialBackup()) &&
      recoveryAllowed())
  {
    buildDummyMooseMesh();

    if (!_app.restoreRestartableDataEarly(restartableName("mfem_par_mesh_state")))
      mooseError("Unable to restore MFEM mesh restartable data '",
                 restartableName("mfem_par_mesh_state"),
                 "'.");

    if (isParamSetByUser("displacement"))
      _mesh_displacement_variable.emplace(getParam<std::string>("displacement"));
  }
  else
    MooseMesh::init();
}

void
MFEMMesh::buildMesh()
{
  TIME_SECTION("buildMesh", 2, "Reading Mesh");

  // Build a dummy MOOSE mesh to enable this class to work with other MOOSE classes.
  buildDummyMooseMesh();

  // Build the MFEM ParMesh from a serial MFEM mesh
  mfem::Mesh mfem_ser_mesh(getFileName());

  if (isParamSetByUser("serial_refine") && isParamSetByUser("uniform_refine"))
    paramError(
        "Cannot define serial_refine and uniform_refine to be nonzero at the same time (they "
        "are the same variable). Please choose one.\n");

  uniformRefinement(mfem_ser_mesh,
                    isParamSetByUser("serial_refine") ? getParam<unsigned int>("serial_refine")
                                                      : getParam<unsigned int>("uniform_refine"));

  // MFEM supports load balancing of parallel non-conforming meshes
  // with a space-filling curve partitioning, and we can improve it
  // by re-ordering the mesh. For now, we only support the Hilbert
  // ordering, although there is one other option.
  if (getParam<bool>("reorder_mesh"))
  {
    mfem::Array<int> ordering;
    mfem_ser_mesh.GetHilbertElementOrdering(ordering);
    mfem_ser_mesh.ReorderElements(ordering);
  }

  // Make sure mesh is in non-conforming mode to enable local refinement of
  // quadrilaterals/hexahedra (c.f. MFEM example 6p). The argument (true/false)
  // determines whether a simplex mesh is considered to be non-conforming.
  if (getParam<bool>("nonconforming"))
    mfem_ser_mesh.EnsureNCMesh(true);

  // multi app should take the mpi comm from moose so is split correctly??
  auto comm = this->comm().get();
  _mfem_par_mesh = std::make_shared<mfem::ParMesh>(comm, mfem_ser_mesh);

  // Perform parallel refinements
  uniformRefinement(*_mfem_par_mesh, getParam<unsigned int>("parallel_refine"));

  if (isParamSetByUser("displacement"))
    _mesh_displacement_variable.emplace(getParam<std::string>("displacement"));
}

void
MFEMMesh::storeRestartableParMesh(std::ostream & stream) const
{
  mooseAssert(_mfem_par_mesh, "MFEM parallel mesh is not initialized");
  _mfem_par_mesh->ParPrint(stream);
}

void
MFEMMesh::loadRestartableParMesh(std::istream & stream)
{
  _mfem_par_mesh = std::make_shared<mfem::ParMesh>(this->comm().get(), stream);
}

void
MFEMMesh::displace(mfem::GridFunction const & displacement)
{
  _mfem_par_mesh->EnsureNodes();
  mfem::GridFunction * nodes = _mfem_par_mesh->GetNodes();

  *nodes += displacement;
}

void
MFEMMesh::buildDummyMooseMesh()
{
  MeshTools::Generation::build_point(static_cast<UnstructuredMesh &>(getMesh()));
}

void
MFEMMesh::uniformRefinement(mfem::Mesh & mesh, const unsigned int nref) const
{
  for (unsigned int i = 0; i < nref; ++i)
    mesh.UniformRefinement();
}

std::unique_ptr<MooseMesh>
MFEMMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}

#endif
