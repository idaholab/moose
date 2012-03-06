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
#include "Moose.h"

// libMesh includes
#include "getpot.h"
#include "mesh_generation.h"
#include "string_to_enum.h"


template<>
InputParameters validParams<GeneratedMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  params.addRequiredParam<int>("dim", "The dimension of the mesh to be generated");
  params.addParam<int>("nx", 1, "Number of elements in the X direction");
  params.addParam<int>("ny", 1, "Number of elements in the Y direction");
  params.addParam<int>("nz", 1, "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
  params.addParam<std::string>("elem_type", "The type of element from libMesh to generate");

  return params;
}

GeneratedMesh::GeneratedMesh(const std::string & name, InputParameters parameters) :
    MooseMesh(name, parameters),
    _dim(getParam<int>("dim")),
    _nx(getParam<int>("nx")),
    _ny(getParam<int>("ny")),
    _nz(getParam<int>("nz")),
    _xmin(getParam<Real>("xmin")),
    _xmax(getParam<Real>("xmax")),
    _ymin(getParam<Real>("ymin")),
    _ymax(getParam<Real>("ymax")),
    _zmin(getParam<Real>("zmin")),
    _zmax(getParam<Real>("zmax"))
{
  std::string elem_type_str;

  if (isParamValid("elem_type"))
    elem_type_str = getParam<std::string>("elem_type");
  else
    switch (_dim)
    {
    case 1: elem_type_str = "EDGE2"; break;
    case 2: elem_type_str = "QUAD4"; break;
    case 3: elem_type_str = "HEX8"; break;
    default:
      mooseError("Unable to generate mesh for unknown dimension");
      break;
    }

  ElemType elem_type = Utility::string_to_enum<ElemType>(elem_type_str);

  switch (_dim)
  {
  case 1:
    MeshTools::Generation::build_line(_mesh, _nx, _xmin, _xmax, elem_type);
    break;
  case 2:
    MeshTools::Generation::build_square(_mesh, _nx, _ny, _xmin, _xmax, _ymin, _ymax, elem_type);
    break;
  case 3:
    MeshTools::Generation::build_cube(_mesh, _nx, _ny, _nz, _xmin, _xmax, _ymin, _ymax, _zmin, _zmax, elem_type);
    break;
  }
}

GeneratedMesh::~GeneratedMesh()
{
}
