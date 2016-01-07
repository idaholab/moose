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

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

template<>
InputParameters validParams<GeneratedMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  MooseEnum elem_types("EDGE EDGE2 EDGE3 EDGE4 QUAD QUAD4 QUAD8 QUAD9 TRI3 TRI6 HEX HEX8 HEX20 HEX27 TET4 TET10 PRISM6 PRISM15 PRISM18 PYRAMID5 PYRAMID13 PYRAMID14"); // no default

  MooseEnum dims("1=1 2 3");
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated"); // Make this parameter required

  params.addRangeCheckedParam<int>("nx", 1, "nx>0", "Number of elements in the X direction");
  params.addRangeCheckedParam<int>("ny", 1, "ny>=0", "Number of elements in the Y direction");
  params.addRangeCheckedParam<int>("nz", 1, "nz>=0", "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
  params.addParam<MooseEnum>("elem_type", elem_types, "The type of element from libMesh to generate (default: linear element for requested dimension)");

  params.addRangeCheckedParam<Real>("bias_x", 1., "bias_x>=0.5 & bias_x<=2", "The amount by which to grow (or shrink) the cells in the x-direction.");
  params.addRangeCheckedParam<Real>("bias_y", 1., "bias_y>=0.5 & bias_y<=2", "The amount by which to grow (or shrink) the cells in the y-direction.");
  params.addRangeCheckedParam<Real>("bias_z", 1., "bias_z>=0.5 & bias_z<=2", "The amount by which to grow (or shrink) the cells in the z-direction.");

  params.addParamNamesToGroup("dim", "Main");

  return params;
}

GeneratedMesh::GeneratedMesh(const InputParameters & parameters) :
    MooseMesh(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(getParam<int>("nx")),
    _ny(getParam<int>("ny")),
    _nz(getParam<int>("nz")),
    _xmin(getParam<Real>("xmin")),
    _xmax(getParam<Real>("xmax")),
    _ymin(getParam<Real>("ymin")),
    _ymax(getParam<Real>("ymax")),
    _zmin(getParam<Real>("zmin")),
    _zmax(getParam<Real>("zmax")),
    _bias_x(getParam<Real>("bias_x")),
    _bias_y(getParam<Real>("bias_y")),
    _bias_z(getParam<Real>("bias_z"))
{
}

GeneratedMesh::GeneratedMesh(const GeneratedMesh & other_mesh) :
    MooseMesh(other_mesh),
    _dim(other_mesh._dim),
    _nx(other_mesh._nx),
    _ny(other_mesh._ny),
    _nz(other_mesh._nz),
    _xmin(getParam<Real>("xmin")),
    _xmax(getParam<Real>("xmax")),
    _ymin(getParam<Real>("ymin")),
    _ymax(getParam<Real>("ymax")),
    _zmin(getParam<Real>("zmin")),
    _zmax(getParam<Real>("zmax")),
    _bias_x(getParam<Real>("bias_x")),
    _bias_y(getParam<Real>("bias_y")),
    _bias_z(getParam<Real>("bias_z"))
{
}

GeneratedMesh::~GeneratedMesh()
{
}

MooseMesh &
GeneratedMesh::clone() const
{
  return *(new GeneratedMesh(*this));
}

void
GeneratedMesh::buildMesh()
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
    // The build_XYZ mesh generation functions take an
    // UnstructuredMesh& as the first argument, hence the dynamic_cast.
  case 1:
    MeshTools::Generation::build_line(dynamic_cast<UnstructuredMesh&>(getMesh()),
                                      _nx,
                                      _xmin, _xmax,
                                      elem_type);
    break;
  case 2:
    MeshTools::Generation::build_square(dynamic_cast<UnstructuredMesh&>(getMesh()),
                                        _nx, _ny,
                                        _xmin, _xmax,
                                        _ymin, _ymax,
                                        elem_type);
    break;
  case 3:
    MeshTools::Generation::build_cube(dynamic_cast<UnstructuredMesh&>(getMesh()),
                                      _nx, _ny, _nz,
                                      _xmin, _xmax,
                                      _ymin, _ymax,
                                      _zmin, _zmax,
                                      elem_type);
    break;
  }

  // Apply the bias if any exists
  if (_bias_x != 1.0 || _bias_y != 1.0 || _bias_z != 1.0)
  {
    // Reference to the libmesh mesh
    MeshBase & mesh = getMesh();

    // Biases
    Real bias[3] = {_bias_x, _bias_y, _bias_z};

    // "width" of the mesh in each direction
    Real width[3] = {_xmax - _xmin, _ymax - _ymin, _zmax - _zmin};

    // Min mesh extent in each direction.
    Real mins[3] = {_xmin, _ymin, _zmin};

    // Number of elements in each direction.
    int nelem[3] = {_nx, _ny, _nz};

    // Loop over the nodes and move them to the desired location
    MeshBase::node_iterator       node_it  = mesh.nodes_begin();
    const MeshBase::node_iterator node_end = mesh.nodes_end();

    for ( ; node_it != node_end; ++node_it)
    {
      Node & node = **node_it;

      for (unsigned int dir=0; dir<3; ++dir)
      {
        if (width[dir] != 0. && bias[dir] != 1.)
        {
          // What "index" we are at in the current direction.  We
          // round to the nearest integral value to get this.
          int index = round((node(dir) - mins[dir]) * nelem[dir] / width[dir]);

          // Move node to biased location.
          node(dir) = mins[dir] + width[dir] * (1. - std::pow(bias[dir], index)) / (1. - std::pow(bias[dir], nelem[dir]));
        }
      }
    }
  }
}
