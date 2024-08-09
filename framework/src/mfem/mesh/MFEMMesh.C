//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

  // Perform serial refinements
  uniformRefinement(&mfem_ser_mesh, getParam<int>("serial_refinements"));

  _mfem_par_mesh = std::make_shared<mfem::ParMesh>(MPI_COMM_WORLD, mfem_ser_mesh);

  // Perform parallel refinements
  uniformRefinement(mfem_par_mesh.get(), getParam<int>("parallel_refinements"));
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

  element->set_node(0) = getMesh().add_point(pt1);
  element->set_node(1) = getMesh().add_point(pt2);
  element->set_node(2) = getMesh().add_point(pt3);
  element->set_node(3) = getMesh().add_point(pt4);

  getMesh().prepare_for_use();
}

void
MFEMMesh::uniformRefinement(mfem::ParMesh * mesh, int nref)
{

  for (int i = 0; i < nref; ++i)
    mesh->UniformRefinement();
}

std::unique_ptr<MooseMesh>
MFEMMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}
