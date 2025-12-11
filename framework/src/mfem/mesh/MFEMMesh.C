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
#include "libmesh/mesh_generation.h"
#include "MFEMPeriodicBCs.h"

registerMooseObject("MooseApp", MFEMMesh);

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

  params.addClassDescription("Class to read in and store an mfem::ParMesh from file.");

  return params;
}

MFEMMesh::MFEMMesh(const InputParameters & parameters) : FileMesh(parameters) {}

MFEMMesh::~MFEMMesh() {}

void
MFEMMesh::buildMesh()
{
  TIME_SECTION("buildMesh", 2, "Reading Mesh");

  // Build a dummy MOOSE mesh to enable this class to work with other MOOSE classes.
  buildDummyMooseMesh();

  // Build the MFEM ParMesh from a serial MFEM mesh
  mfem::Mesh mfem_ser_mesh(getFileName());

  if (_periodic) {
    mfem_ser_mesh = applyPeriodicBoundaryByTranslation(mfem_ser_mesh);
  }

  if (isParamSetByUser("serial_refine") && isParamSetByUser("uniform_refine"))
    paramError(
        "Cannot define serial_refine and uniform_refine to be nonzero at the same time (they "
        "are the same variable). Please choose one.\n");

  uniformRefinement(mfem_ser_mesh,
                    isParamSetByUser("serial_refine") ? getParam<unsigned int>("serial_refine")
                                                      : getParam<unsigned int>("uniform_refine"));

  // multi app should take the mpi comm from moose so is split correctly??
  auto comm = this->comm().get();
  _mfem_par_mesh = std::make_shared<mfem::ParMesh>(comm, mfem_ser_mesh);

  // Perform parallel refinements
  uniformRefinement(*_mfem_par_mesh, getParam<unsigned int>("parallel_refine"));

  if (isParamSetByUser("displacement"))
  {
    _mesh_displacement_variable.emplace(getParam<std::string>("displacement"));
  }
}

void
MFEMMesh::registerPeriodicBCs(MFEMPeriodicByVector & bc)
{
  _periodic = true;
  _translation_x = bc.GetPeriodicBc(0);
  _translation_y = bc.GetPeriodicBc(1);
  if (bc.Use3D())
  {
    _translation_z = bc.GetPeriodicBc(2);
  }
}

mfem::Mesh
MFEMMesh::applyPeriodicBoundaryByTranslation(mfem::Mesh& input) {
  std::vector<mfem::Vector> translations(input.SpaceDimension());

  // // error checking. Demand that the z array is set by the user
  mooseAssert((_translation_x.Size() == input.SpaceDimension()) and
                  (_translation_y.Size() == input.SpaceDimension()),
              "The translation vectors must be all set by user if using periodic BCs");
  mooseAssert((input.SpaceDimension() == _translation_z.Size()) or (_translation_z.Size() == 0),
              "Asked for 3D but didn't set the z vector");

  translations[0] = _translation_x;
  translations[1] = _translation_y;

  if (input.SpaceDimension() == 3)
  {
    translations[2] = _translation_z;
  }

  return mfem::Mesh::MakePeriodic(input, input.CreatePeriodicVertexMapping(translations));
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
