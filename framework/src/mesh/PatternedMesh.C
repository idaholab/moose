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

#include "PatternedMesh.h"
#include "Parser.h"
#include "InputParameters.h"

// libMesh includes
#include "libmesh/mesh_modification.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/exodusII_io.h"

template<>
InputParameters validParams<PatternedMesh>()
{
  InputParameters params = validParams<MooseMesh>();
  params.addRequiredParam<std::vector<MeshFileName> >("files", "The name of the mesh files to read.  They are automatically assigned ids starting with zero.");

  params.addRangeCheckedParam<Real>("x_width", 0, "x_width>=0", "The tile width in the x direction");
  params.addRangeCheckedParam<Real>("y_width", 0, "y_width>=0", "The tile width in the y direction");
  params.addRangeCheckedParam<Real>("z_width", 0, "z_width>=0", "The tile width in the z direction");

  // x boundary names
  params.addParam<BoundaryName>("left_boundary", "left_boundary", "name of the left (x) boundary");
  params.addParam<BoundaryName>("right_boundary", "right_boundary", "name of the right (x) boundary");

  // y boundary names
  params.addParam<BoundaryName>("top_boundary", "top_boundary", "name of the top (y) boundary");
  params.addParam<BoundaryName>("bottom_boundary", "bottom_boundary", "name of the bottom (y) boundary");

  params.addRequiredParam<std::vector<std::vector<unsigned int> > >("pattern", "A double-indexed array starting with the upper-left corner");

  params.addClassDescription("Creates a 2D mesh from a specified set of unique 'tiles' meshes and a two-dimensional pattern.");

  return params;
}

PatternedMesh::PatternedMesh(const InputParameters & parameters) :
    MooseMesh(parameters),
    _files(getParam<std::vector<MeshFileName> >("files")),
    _pattern(getParam<std::vector<std::vector<unsigned int> > >("pattern")),
    _x_width(getParam<Real>("x_width")),
    _y_width(getParam<Real>("y_width")),
    _z_width(getParam<Real>("z_width"))
{
  // The PatternedMesh class only works with SerialMesh
  errorIfParallelDistribution("PatternedMesh");

  _meshes.resize(_files.size());

  // Read in all of the meshes
  for (unsigned int i = 0; i < _files.size(); i++)
  {
    SerialMesh * mesh = new SerialMesh(_communicator);
    mesh->read(_files[i]);

    _meshes[i] = mesh;
  }

  _row_meshes.resize(_pattern.size());

  // The zeroth row (which is the top row) will be the real mesh
  _row_meshes[0] = dynamic_cast<SerialMesh*>( &getMesh() );

  // Create a mesh for all the other rows
  for (unsigned int i = 1; i < _pattern.size(); i++)
  {
    SerialMesh * mesh = new SerialMesh(_communicator);
    _row_meshes[i] = mesh;
  }
}

PatternedMesh::PatternedMesh(const PatternedMesh & other_mesh) :
    MooseMesh(other_mesh),
    _files(other_mesh._files),
    _pattern(other_mesh._pattern),
    _x_width(other_mesh._x_width),
    _y_width(other_mesh._y_width),
    _z_width(other_mesh._z_width)
{
}

PatternedMesh::~PatternedMesh()
{
  // Clean up the mesh we made (see what I did there?)
  for (unsigned int i = 0; i < _meshes.size(); i++)
    delete _meshes[i];

  // Don't delete the first row - it is the real mesh - it will be cleaned up later
  for (unsigned int i = 1; i < _row_meshes.size(); i++)
    delete _row_meshes[i];
}


MooseMesh &
PatternedMesh::clone() const
{
  return *(new PatternedMesh(*this));
}

void
PatternedMesh::buildMesh()
{
  // stitch_meshes() is only implemented for SerialMesh.  So make sure
  // we have one here before continuing.
  SerialMesh * the_mesh = dynamic_cast<SerialMesh*>(&getMesh());

  if (!the_mesh)
    mooseError("Error, PatternedMesh calls stitch_meshes() which only works on SerialMesh.");
  else
  {
    BoundaryID left = getBoundaryID(getParam<BoundaryName>("left_boundary"));
    BoundaryID right = getBoundaryID(getParam<BoundaryName>("right_boundary"));
    BoundaryID top = getBoundaryID(getParam<BoundaryName>("top_boundary"));
    BoundaryID bottom = getBoundaryID(getParam<BoundaryName>("bottom_boundary"));

    // Build each row mesh
    for (unsigned int i = 0; i < _pattern.size(); i++)
      for (unsigned int j = 0; j < _pattern[i].size(); j++)
      {
        Real
          deltax = j*_x_width,
          deltay = i*_y_width;

        // If this is the first cell of the row initialize the row mesh
        if (j == 0)
        {
          _row_meshes[i]->read(_files[_pattern[i][j]]);

          MeshTools::Modification::translate(*_row_meshes[i], deltax, -deltay, 0);

          continue;
        }

        SerialMesh & cell_mesh = *_meshes[_pattern[i][j]];

        // Move the mesh into the right spot.  -i because we are starting at the top
        MeshTools::Modification::translate(cell_mesh, deltax, -deltay, 0);

        _row_meshes[i]->stitch_meshes(dynamic_cast<SerialMesh &>(cell_mesh), right, left, TOLERANCE, /*clear_stitched_boundary_ids=*/true);

        // Undo the translation
        MeshTools::Modification::translate(cell_mesh, -deltax, deltay, 0);
      }

    // Now stitch together the rows
    // We're going to stitch them all to row 0 (which is the real mesh)
    for (unsigned int i = 1; i < _pattern.size(); i++)
      _row_meshes[0]->stitch_meshes(*_row_meshes[i], bottom, top, TOLERANCE, /*clear_stitched_boundary_ids=*/true);
  }
}
