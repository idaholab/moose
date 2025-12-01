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

  params.addParam<bool>("periodic", false, "Optional variable to indicate whether we make the mesh periodic.");
  params.addParam<std::vector<Real>>("translation_x",
                                     "Vector specifying translation in x direction.");
  params.addParam<std::vector<Real>>("translation_y",
                                     "Vector specifying translation in y direction.");
  params.addParam<std::vector<Real>>("translation_z",
                                     "Vector specifying translation in z direction.");

  params.addClassDescription("Class to read in and store an mfem::ParMesh from file.");

  return params;
}

MFEMMesh::MFEMMesh(const InputParameters & parameters) : FileMesh(parameters),
  _periodic(getParam<bool>("periodic"))
{
  if (_periodic) {
    // hardcode to fetch x and y
    _translation_x = getParam<std::vector<Real>>("translation_x");
    _translation_y = getParam<std::vector<Real>>("translation_y");
    if (isParamSetByUser("translation_z"))
      _translation_z = getParam<std::vector<Real>>("translation_z");
  }
}

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

mfem::Mesh
MFEMMesh::applyPeriodicBoundaryByTranslation(mfem::Mesh& input) {
  std::vector<mfem::Vector> translations(input.SpaceDimension());

  // error checking. Demand that the z array is set by the user
  mooseAssert((_translation_x.size() == input.SpaceDimension()) and
                  (_translation_y.size() == input.SpaceDimension()),
              "The translation vectors must be all set by user if using periodic BCs");
  mooseAssert((input.SpaceDimension() == _translation_z.size()) or (_translation_z.size() == 0),
              "Asked for 3D but didn't set the z vector");

  if (input.SpaceDimension() == 2)
  {
    mfem::Vector x(_translation_x.data(), 2);
    mfem::Vector y(_translation_y.data(), 2);

    translations[0] = x;
    translations[1] = y;
    return mfem::Mesh::MakePeriodic(input, input.CreatePeriodicVertexMapping(translations));
  }

  else if (input.SpaceDimension() == 3)
  {
    mfem::Vector x(_translation_x.data(), 3);
    mfem::Vector y(_translation_y.data(), 3);
    mfem::Vector z(_translation_z.data(), 3);

    translations[0] = x;
    translations[1] = y;
    translations[2] = z;
    return mfem::Mesh::MakePeriodic(input, input.CreatePeriodicVertexMapping(translations));
  }

  else
  {
    mooseError("Bad value for dimension!");
  }
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
