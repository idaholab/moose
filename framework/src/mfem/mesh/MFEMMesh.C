#ifdef MFEM_ENABLED

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MFEMMesh.h"

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

  if (isParamSetByUser("serial_refine") && isParamSetByUser("uniform_refine"))
    paramError(
        "Cannot define serial_refine and uniform_refine to be nonzero at the same time (they "
        "are the same variable). Please choose one.\n");

  uniformRefinement(mfem_ser_mesh,
                    isParamSetByUser("serial_refine") ? getParam<unsigned int>("serial_refine")
                                                      : getParam<unsigned int>("uniform_refine"));

  // multi app should take the mpi comm from moose so is split correctly??
  auto comm = _app.comm().get();
  _mfem_par_mesh = std::make_shared<mfem::ParMesh>(comm, mfem_ser_mesh);

  // Perform parallel refinements
  uniformRefinement(*_mfem_par_mesh, getParam<unsigned int>("parallel_refine"));

  if (isParamSetByUser("displacement"))
  {
    _mesh_displacement_variable.emplace(getParam<std::string>("displacement"));
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
  auto element = new Quad4;
  element->set_id() = 1;
  element->processor_id() = 0;

  getMesh().add_elem(element);

  Point pt1(0.0, 0.0, 0.0);
  Point pt2(1.0, 0.0, 0.0);
  Point pt3(1.0, 1.0, 0.0);
  Point pt4(0.0, 1.0, 0.0);

  element->set_node(0, getMesh().add_point(pt1));
  element->set_node(1, getMesh().add_point(pt2));
  element->set_node(2, getMesh().add_point(pt3));
  element->set_node(3, getMesh().add_point(pt4));

  getMesh().prepare_for_use();
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
