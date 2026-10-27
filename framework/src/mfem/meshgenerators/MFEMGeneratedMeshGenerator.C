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

  // The MooseEnum values are set to match mfem::Element::Type so that
  // getEnum<mfem::Element::Type>() can be used directly below, while the names correspond to the
  // form MOOSE users of libMesh-based meshes are already familiar with. POINT, WEDGE, and PYRAMID
  // are omitted: mfem::Mesh::MakeCartesian1D/2D/3D, which this generator uses, only ever produce an
  // edge/segment (1D), a triangle/quadrilateral (2D), or a tetrahedron/hexahedron (3D).
  MooseEnum elem_types("EDGE=1 TRI=2 QUAD=3 TET=4 HEX=5");
  params.addParam<MooseEnum>("elem_type",
                             elem_types,
                             "Element type. Use EDGE for 1D meshes, TRI or QUAD for 2D meshes, "
                             "TET or HEX for 3D meshes. If not specified, defaults to EDGE for "
                             "1D, QUAD for 2D, and HEX for 3D.");

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
    _zmax(getParam<Real>("zmax")),
    _elem_type(
        [this]()
        {
          // Apply dimension-dependent default for elem_type if not set by user
          if (!isParamSetByUser("elem_type"))
          {
            if (_dim == 1)
              return mfem::Element::SEGMENT;
            else if (_dim == 2)
              return mfem::Element::QUADRILATERAL;
            return mfem::Element::HEXAHEDRON;
          }

          const auto elem_type = getParam<MooseEnum>("elem_type").getEnum<mfem::Element::Type>();
          if ((_dim == 1 && elem_type != mfem::Element::SEGMENT) ||
              (_dim == 2 && elem_type != mfem::Element::TRIANGLE &&
               elem_type != mfem::Element::QUADRILATERAL) ||
              (_dim == 3 && elem_type != mfem::Element::TETRAHEDRON &&
               elem_type != mfem::Element::HEXAHEDRON))
            paramError("elem_type",
                       "Use EDGE for 1D meshes, TRI or QUAD for 2D meshes, "
                       "and TET or HEX for 3D meshes.");
          return elem_type;
        }())
{
}

namespace
{
void
addBdrSet(mfem::Mesh & mesh, int attr, const std::string & name)
{
  mesh.bdr_attribute_sets.SetAttributeSet(name, mfem::Array<int>{attr});
}
} // namespace

std::unique_ptr<mfem::Mesh>
MFEMGeneratedMeshGenerator::generateMFEMMesh()
{
  if (_dim == 1)
  {
    auto mesh = std::make_unique<mfem::Mesh>(mfem::Mesh::MakeCartesian1D(_nx, _xmax));
    addBdrSet(*mesh, 1, "left");
    addBdrSet(*mesh, 2, "right");
    return mesh;
  }

  if (_dim == 2)
  {
    auto mesh = std::make_unique<mfem::Mesh>(
        mfem::Mesh::MakeCartesian2D(_nx, _ny, _elem_type, true, _xmax, _ymax));
    addBdrSet(*mesh, 1, "bottom");
    addBdrSet(*mesh, 2, "right");
    addBdrSet(*mesh, 3, "top");
    addBdrSet(*mesh, 4, "left");
    return mesh;
  }

  // dim == 3
  auto mesh = std::make_unique<mfem::Mesh>(
      mfem::Mesh::MakeCartesian3D(_nx, _ny, _nz, _elem_type, _xmax, _ymax, _zmax));
  addBdrSet(*mesh, 1, "bottom");
  addBdrSet(*mesh, 2, "front");
  addBdrSet(*mesh, 3, "right");
  addBdrSet(*mesh, 4, "back");
  addBdrSet(*mesh, 5, "left");
  addBdrSet(*mesh, 6, "top");
  return mesh;
}

#endif
