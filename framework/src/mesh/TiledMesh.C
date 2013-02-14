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

#include "TiledMesh.h"
#include "Parser.h"
#include "InputParameters.h"

// libMesh includes
#include "libmesh/mesh_modification.h"

template<>
InputParameters validParams<TiledMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  params.addParam<Real>("x_width", 0, "The tile width in the x direction");
  params.addParam<Real>("y_width", 0, "The tile width in the y direction");
  params.addParam<Real>("z_width", 0, "The tile width in the z direction");

  params.addParam<BoundaryName>("left_boundary", "TODO");
  params.addParam<BoundaryName>("right_boundary", "TODO");
  params.addParam<BoundaryName>("top_boundary", "TODO");
  params.addParam<BoundaryName>("bottom_boundary", "TODO");
  params.addParam<BoundaryName>("front_boundary", "TODO");
  params.addParam<BoundaryName>("back_boundary", "TODO");

  params.addParam<unsigned int>("x_tiles", 1, "TODO");
  params.addParam<unsigned int>("y_tiles", 1, "TODO");
  params.addParam<unsigned int>("z_tiles", 1, "TODO");

  return params;
}

TiledMesh::TiledMesh(const std::string & name, InputParameters parameters):
    MooseMesh(name, parameters),
    _x_width(getParam<Real>("x_width")),
    _y_width(getParam<Real>("y_width")),
    _z_width(getParam<Real>("z_width"))
{

// This class only works with SerialMesh
#ifndef LIBMESH_ENABLE_PARMESH

  std::string mesh_file(getParam<MeshFileName>("file"));

  if (mesh_file.rfind(".exd") < mesh_file.size() ||
      mesh_file.rfind(".e") < mesh_file.size())
  {
    ExodusII_IO ex(*this);
    ex.read(mesh_file);
    _mesh.prepare_for_use();
  }
  else
    read(mesh_file);

  BoundaryID left = getBoundaryID(getParam<BoundaryName>("left_boundary"));
  BoundaryID right = getBoundaryID(getParam<BoundaryName>("right_boundary"));
  BoundaryID top = getBoundaryID(getParam<BoundaryName>("top_boundary"));
  BoundaryID bottom = getBoundaryID(getParam<BoundaryName>("bottom_boundary"));
  BoundaryID front = getBoundaryID(getParam<BoundaryName>("front_boundary"));
  BoundaryID back = getBoundaryID(getParam<BoundaryName>("back_boundary"));

  {
    AutoPtr<MeshBase> clone = _mesh.clone();

    // Build X Tiles
    for (unsigned int i=1; i<getParam<unsigned int>("x_tiles"); ++i)
    {
      MeshTools::Modification::translate(*clone, _x_width, 0, 0);
      _mesh.stitch_meshes(dynamic_cast<SerialMesh &>(*clone), right, left, TOLERANCE, true);
    }
  }
  {
    AutoPtr<MeshBase> clone = _mesh.clone();

    // Build Y Tiles
    for (unsigned int i=1; i<getParam<unsigned int>("y_tiles"); ++i)
    {
      MeshTools::Modification::translate(*clone, 0, _y_width, 0);
      _mesh.stitch_meshes(dynamic_cast<SerialMesh &>(*clone), top, bottom, TOLERANCE, true);
    }
  }
  {
    AutoPtr<MeshBase> clone = _mesh.clone();

    // Build Y Tiles
    for (unsigned int i=1; i<getParam<unsigned int>("z_tiles"); ++i)
    {
      MeshTools::Modification::translate(*clone, 0, 0, _z_width);
      _mesh.stitch_meshes(dynamic_cast<SerialMesh &>(*clone), front, back, TOLERANCE, true);
    }
  }

#else
  mooseError("TiledMesh cannot be used with --enable-parmesh");
#endif

}
