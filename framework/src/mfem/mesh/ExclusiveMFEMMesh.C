//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExclusiveMFEMMesh.h"

registerMooseObject("MooseApp", ExclusiveMFEMMesh);

InputParameters
ExclusiveMFEMMesh::validParams()
{
  InputParameters params = FileMesh::validParams();
  return params;
}

ExclusiveMFEMMesh::ExclusiveMFEMMesh(const InputParameters & parameters) : FileMesh(parameters) {}

ExclusiveMFEMMesh::~ExclusiveMFEMMesh() {}

void
ExclusiveMFEMMesh::buildMesh()
{
  // Build a dummy MOOSE mesh to enable this class to work with other MOOSE classes.
  buildDummyMesh();
}

void
ExclusiveMFEMMesh::buildDummyMesh()
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

void
ExclusiveMFEMMesh::buildMFEMMesh()
{
  _mfem_mesh = std::make_shared<MFEMMesh>(getFileName());
}

void
ExclusiveMFEMMesh::buildMFEMParMesh()
{
  _mfem_par_mesh = std::make_shared<MFEMParMesh>(MPI_COMM_WORLD, getMFEMMesh());
  _mfem_mesh.reset(); // Lower reference count of serial mesh since no longer needed.
}

MFEMMesh &
ExclusiveMFEMMesh::getMFEMMesh()
{
  if (!_mfem_mesh)
  {
    buildMFEMMesh();
  }

  return *_mfem_mesh;
}

MFEMParMesh &
ExclusiveMFEMMesh::getMFEMParMesh()
{
  if (!_mfem_par_mesh)
  {
    buildMFEMParMesh();
  }

  return *_mfem_par_mesh;
}

std::unique_ptr<MooseMesh>
ExclusiveMFEMMesh::safeClone() const
{
  return std::make_unique<ExclusiveMFEMMesh>(*this);
}
