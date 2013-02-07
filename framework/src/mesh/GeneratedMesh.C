/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "GeneratedMesh.h"
#include "NonlinearSystem.h"

// libMesh includes
#include "libmesh/getpot.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary_base.h"

template<>
InputParameters validParams<GeneratedMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  MooseEnum elem_types("EDGE, EDGE2, EDGE3, EDGE4, QUAD, QUAD4, QUAD8, QUAD9, TRI3, TRI6, HEX, HEX8, HEX20, HEX27, TET4, TET10"); // no default
  MooseEnum dims("1 = 1, 2, 3");

  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated");
  params.addParam<int>("nx", 1, "Number of elements in the X direction");
  params.addParam<int>("ny", 1, "Number of elements in the Y direction");
  params.addParam<int>("nz", 1, "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
  params.addParam<MooseEnum>("elem_type", elem_types, "The type of element from libMesh to generate (default: linear element for requested dimension)");
  params.addPrivateParam<std::string>("built_by_action", "read_mesh");

  params.addParamNamesToGroup("dim", "Main");

  return params;
}

GeneratedMesh::GeneratedMesh(const std::string & name, InputParameters parameters) :
    MooseMesh(name, parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(getParam<int>("nx")),
    _ny(getParam<int>("ny")),
    _nz(getParam<int>("nz"))
{
  MooseEnum elem_type_enum = getParam<MooseEnum>("elem_type");

  if (!isParamValid("elem_type"))
  {
    // Switching on MooseEnum
    switch (_dim)
    {
    case 1: elem_type_enum = "EDGE2"; break;
    case 2: elem_type_enum = "QUAD4"; break;
    case 3: elem_type_enum = "HEX8"; break;
    }
  }

  ElemType elem_type = Utility::string_to_enum<ElemType>(elem_type_enum);

  // Switching on MooseEnum
  switch (_dim)
  {
  case 1:
    MeshTools::Generation::build_line(_mesh, _nx, getParam<Real>("xmin"), getParam<Real>("xmax"), elem_type);
    break;
  case 2:
    MeshTools::Generation::build_square(_mesh, _nx, _ny, getParam<Real>("xmin"), getParam<Real>("xmax"), getParam<Real>("ymin"), getParam<Real>("ymax"), elem_type);
    break;
  case 3:
    MeshTools::Generation::build_cube(_mesh, _nx, _ny, _nz, getParam<Real>("xmin"), getParam<Real>("xmax"), getParam<Real>("ymin"), getParam<Real>("ymax"), getParam<Real>("zmin"), getParam<Real>("zmax"), elem_type);
    break;
  }
}

GeneratedMesh::~GeneratedMesh()
{
}
