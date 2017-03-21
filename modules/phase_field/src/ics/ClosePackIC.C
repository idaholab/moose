/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "ClosePackIC.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/mesh_tools.h"

template <>
InputParameters
validParams<ClosePackIC>()
{
  InputParameters params = validParams<SmoothCircleBaseIC>();
  params.addClassDescription("Close packed arrangement of smooth circles");
  params.addRequiredParam<Real>("radius", "The radius of a circle");
  return params;
}

ClosePackIC::ClosePackIC(const InputParameters & parameters)
  : SmoothCircleBaseIC(parameters), _radius(parameters.get<Real>("radius"))
{
}

void
ClosePackIC::computeCircleCenters()
{
  // Determine the extents of the mesh
  MeshTools::BoundingBox bbox = MeshTools::bounding_box(_fe_problem.mesh().getMesh());
  const Point & min = bbox.min();
  const Point & max = bbox.max();

  // Create the x,y,z limits for the while loops
  Real x_min = min(0);
  Real x_max = max(0) + 2.0 * _radius;

  Real y_min = min(1) - 2.0 * std::sqrt(3.0) * _radius + _radius;
  Real y_max = max(1) + 2.0 * _radius;

  // Real z_min = min(2) - 2*std::sqrt(3.0)*_radius + _radius;
  Real z_max = 0.0;

  // Initialize the coordinates that will be used in looping
  Real x = x_min;
  Real y = y_min;
  Real z = 0.0;

  // Adjust the 3D z-dimension maximum
  if (_fe_problem.mesh().dimension() == 3)
    z_max = max(2) + 2.0 * _radius;

  // Counters for offsetting every other row column in x,y dimensions
  unsigned int i = 0;
  unsigned int j = 0;

  while (z <= z_max)
  {
    // Offset the y-coordinate by sqrt(3)*r every other loop
    if (j % 2 != 0)
      y += std::sqrt(3) * _radius / 2.0;

    while (y <= y_max)
    {

      // Offset the x-coordinate by r every other loop
      if (i % 2 == 0)
        x += _radius;

      while (x <= x_max)
      {
        _centers.push_back(Point(x, y, z));
        _radii.push_back(_radius);
        x += 2.0 * _radius;
      }

      // Reset x-coord and increment y-coord
      x = x_min;
      y += std::sqrt(3.0) * _radius;
      i++;
    }

    // Reset y-coord and increment z-coord
    y = y_min;
    z += std::sqrt(3.0) * _radius;
    j++;
  }
}
