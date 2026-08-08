//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMGeneratedMeshGenerator.h"

registerMooseObject("MooseApp", MFEMGeneratedMeshGenerator);

InputParameters
MFEMGeneratedMeshGenerator::validParams()
{
  InputParameters params = MFEMMeshGenerator::validParams();

  MooseEnum dims("1=1 2=2 3=3");
  params.addRequiredParam<MooseEnum>("dim", dims, "Spatial dimension of the mesh (1, 2, or 3).");

  params.addParam<unsigned int>("nx", 1, "Number of elements in the x direction.");
  params.addParam<unsigned int>("ny", 1, "Number of elements in the y direction.");
  params.addParam<unsigned int>("nz", 1, "Number of elements in the z direction.");

  params.addParam<Real>("xmax", 1.0, "Upper bound of the domain in the x direction.");
  params.addParam<Real>("ymax", 1.0, "Upper bound of the domain in the y direction.");
  params.addParam<Real>("zmax", 1.0, "Upper bound of the domain in the z direction.");

  // 2D default is QUAD; 3D default is HEX. The valid set covers all four options so
  // the same parameter works for any dimension, with validation deferred to generateMFEMMesh().
  MooseEnum elem_types("QUAD=0 TRI=1 HEX=2 TET=3", "QUAD");
  params.addParam<MooseEnum>(
      "elem_type",
      elem_types,
      "Element type. Use QUAD or TRI for 2D meshes, HEX or TET for 3D meshes.");

  params.addClassDescription("Generates a structured Cartesian MFEM mesh (line, rectangle, or box) "
                             "with uniformly spaced elements.");

  return params;
}

MFEMGeneratedMeshGenerator::MFEMGeneratedMeshGenerator(const InputParameters & parameters)
  : MFEMMeshGenerator(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _nz(getParam<unsigned int>("nz")),
    _xmax(getParam<Real>("xmax")),
    _ymax(getParam<Real>("ymax")),
    _zmax(getParam<Real>("zmax"))
{
}

namespace
{
void
addBdrSet(mfem::Mesh & mesh, int attr, const std::string & name)
{
  mfem::Array<int> a;
  a.Append(attr);
  mesh.bdr_attribute_sets.SetAttributeSet(name, a);
}
} // namespace

mfem::Mesh
MFEMGeneratedMeshGenerator::generateMFEMMesh()
{
  if (_dim == 1)
  {
    auto mesh = mfem::Mesh::MakeCartesian1D(_nx, _xmax);
    addBdrSet(mesh, 1, "left");
    addBdrSet(mesh, 2, "right");
    return mesh;
  }

  const MooseEnum & elem_type_param = getParam<MooseEnum>("elem_type");

  if (_dim == 2)
  {
    if (elem_type_param == "HEX" || elem_type_param == "TET")
      paramError("elem_type", "Use QUAD or TRI for 2D meshes.");

    const auto elem_type =
        (elem_type_param == "TRI") ? mfem::Element::TRIANGLE : mfem::Element::QUADRILATERAL;
    auto mesh = mfem::Mesh::MakeCartesian2D(_nx, _ny, elem_type, true, _xmax, _ymax);
    addBdrSet(mesh, 1, "bottom");
    addBdrSet(mesh, 2, "right");
    addBdrSet(mesh, 3, "top");
    addBdrSet(mesh, 4, "left");
    return mesh;
  }

  // dim == 3
  if (elem_type_param == "QUAD" || elem_type_param == "TRI")
    paramError("elem_type", "Use HEX or TET for 3D meshes.");

  const auto elem_type =
      (elem_type_param == "TET") ? mfem::Element::TETRAHEDRON : mfem::Element::HEXAHEDRON;
  auto mesh = mfem::Mesh::MakeCartesian3D(_nx, _ny, _nz, elem_type, _xmax, _ymax, _zmax);
  addBdrSet(mesh, 1, "bottom");
  addBdrSet(mesh, 2, "front");
  addBdrSet(mesh, 3, "right");
  addBdrSet(mesh, 4, "back");
  addBdrSet(mesh, 5, "left");
  addBdrSet(mesh, 6, "top");
  return mesh;
}

#endif
