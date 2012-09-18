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
#include "getpot.h"
#include "mesh_generation.h"
#include "string_to_enum.h"
#include "periodic_boundaries.h"
#include "periodic_boundary_base.h"

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

  return params;
}

GeneratedMesh::GeneratedMesh(const std::string & name, InputParameters parameters) :
    MooseMesh(name, parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(getParam<int>("nx")),
    _ny(getParam<int>("ny")),
    _nz(getParam<int>("nz")),
    _periodic_dim(LIBMESH_DIM, false)
{
  _bounds.resize(3);
  for (unsigned int i=0; i<_bounds.size(); ++i)
    _bounds[i].resize(2);

  _bounds[X][MIN] = getParam<Real>("xmin");
  _bounds[X][MAX] = getParam<Real>("xmax");
  _bounds[Y][MIN] = getParam<Real>("ymin");
  _bounds[Y][MAX] = getParam<Real>("ymax");
  _bounds[Z][MIN] = getParam<Real>("zmin");
  _bounds[Z][MAX] = getParam<Real>("zmax");

  MooseEnum elem_type_enum = getParam<MooseEnum>("elem_type");

  if (!isParamValid("elem_type"))
  {
    // Switching on MooseEnum
    switch (_dim)
    {
    case 1: elem_type_enum = "EDGE2"; break;
    case 2: elem_type_enum = "QUAD4"; break;
    case 3: elem_type_enum = "HEX8"; break;
    default:
      mooseError("Unable to generate mesh for unknown dimension");
      break;
    }
  }

  ElemType elem_type = Utility::string_to_enum<ElemType>(elem_type_enum);

  // Switching on MooseEnum
  switch (_dim)
  {
  case 1:
    MeshTools::Generation::build_line(_mesh, _nx, _bounds[X][MIN], _bounds[X][MAX], elem_type);
    break;
  case 2:
    MeshTools::Generation::build_square(_mesh, _nx, _ny, _bounds[X][MIN], _bounds[X][MAX], _bounds[Y][MIN], _bounds[Y][MAX], elem_type);
    break;
  case 3:
    MeshTools::Generation::build_cube(_mesh, _nx, _ny, _nz, _bounds[X][MIN], _bounds[X][MAX], _bounds[Y][MIN], _bounds[Y][MAX], _bounds[Z][MIN], _bounds[Z][MAX], elem_type);
    break;
  }
}

GeneratedMesh::~GeneratedMesh()
{
}

Real
GeneratedMesh::dimensionWidth(unsigned int component) const
{
  mooseAssert(component < LIBMESH_DIM, "Requested dimension out of bounds");

  return _bounds[component][MAX] - _bounds[component][MIN];
}

Real
GeneratedMesh::getMinInDimension(unsigned int component) const
{
  mooseAssert(component < LIBMESH_DIM, "Requested dimension out of bounds");

  return _bounds[component][MIN];
}

Real
GeneratedMesh::getMaxInDimension(unsigned int component) const
{
  mooseAssert(component < LIBMESH_DIM, "Requested dimension out of bounds");

  return _bounds[component][MAX];
}

bool
GeneratedMesh::isPeriodic(NonlinearSystem &nl, unsigned int var_num, unsigned int component)
{
  mooseAssert(component < LIBMESH_DIM, "Requested dimension out of bounds");

  PeriodicBoundaries *pbs = nl.dofMap().get_periodic_boundaries();

  static const int pb_map[3][3] = {{0, -99, -99},{3, 0, -99},{4, 1, 5}};

  PeriodicBoundaryBase *pb = pbs->boundary(pb_map[_mesh.mesh_dimension()-1][component]);

  if (pb != NULL)
    return pb->is_my_variable(var_num);
  else
    return false;
}

void
GeneratedMesh::initPeriodicDistanceForVariable(NonlinearSystem &nl, unsigned int var_num)
{
  for (unsigned int i=0; i<dimension(); ++i)
    _periodic_dim[i] = isPeriodic(nl, var_num, i);

  _half_range = Point(dimensionWidth(0)/2.0, dimensionWidth(1)/2.0, dimensionWidth(2)/2.0);
}

Real
GeneratedMesh::minPeriodicDistance(Point p, Point q) const
{
  for (unsigned int i=0; i<LIBMESH_DIM; ++i)
  {
    // check to see if we're closer in real or periodic space in x, y, and z
    if (_periodic_dim[i])
    {
      // Need to test order before differencing
      if (p(i) > q(i))
      {
        if (p(i) - q(i) > _half_range(i))
          p(i) -= _half_range(i) * 2;
      }
      else
      {
        if (q(i) - p(i) > _half_range(i))
          p(i) += _half_range(i) * 2;
      }
    }
  }

  return (p-q).size();
}

