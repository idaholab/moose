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
  _mfem_par_mesh = std::make_shared<mfem::ParMesh>(MPI_COMM_WORLD, mfem_ser_mesh);
}

void
MFEMMesh::buildDummyMooseMesh()
{
  auto element = new Quad4;
  element->set_id() = 1;
  element->processor_id() = 0;

  _mesh->add_elem(element);

  Point pt1(0.0, 0.0, 0.0);
  Point pt2(1.0, 0.0, 0.0);
  Point pt3(1.0, 1.0, 0.0);
  Point pt4(0.0, 1.0, 0.0);

  element->set_node(0) = _mesh->add_point(pt1);
  element->set_node(1) = _mesh->add_point(pt2);
  element->set_node(2) = _mesh->add_point(pt3);
  element->set_node(3) = _mesh->add_point(pt4);

  _mesh->prepare_for_use();
}

std::unique_ptr<MooseMesh>
MFEMMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}
