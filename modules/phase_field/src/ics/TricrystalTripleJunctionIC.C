//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TricrystalTripleJunctionIC.h"
#include "MooseRandom.h"
#include "MooseMesh.h"
#include "MathUtils.h"

registerMooseObject("PhaseFieldApp", TricrystalTripleJunctionIC);

InputParameters
TricrystalTripleJunctionIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription("Tricrystal with a triple junction");
  params.addRequiredParam<unsigned int>("op_num", "Number of grain order parameters");
  params.addRequiredParam<unsigned int>("op_index", "Index for the current grain order parameter");
  params.addParam<Real>("theta1", 135.0, "Angle of first grain at triple junction in degrees");
  params.addParam<Real>("theta2", 135.0, "Angle of second grain at triple junction in degrees");
  params.addParam<Point>(
      "junction",
      "The point where the triple junction is located. Default is the center of the mesh");
  return params;
}

TricrystalTripleJunctionIC::TricrystalTripleJunctionIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mesh(_fe_problem.mesh()),
    _op_num(getParam<unsigned int>("op_num")),
    _op_index(getParam<unsigned int>("op_index")),
    _theta1(getParam<Real>("theta1")),
    _theta2(getParam<Real>("theta2"))
{
  if (_op_num != 3)
    paramError("op_num", "Tricrystal ICs must have op_num = 3");

  if (_theta1 + _theta2 >= 360.0)
    paramError("theta1", "Sum of the angles theta1 and theta2 must total less than 360 degrees");

  // Default junction point is the center
  if (!parameters.isParamValid("junction"))
  {
    for (const auto i : make_range(Moose::dim))
      _junction(i) = (_mesh.getMaxInDimension(i) - _mesh.getMinInDimension(i)) / 2.0;
  }
  else
    _junction = getParam<Point>("junction");

  // Make sure that _junction is in the domain
  for (const auto i : make_range(Moose::dim))
  {
    if ((_mesh.getMinInDimension(i) > _junction(i)) || (_mesh.getMaxInDimension(i) < _junction(i)))
      paramError("junction", "Triple junction out of bounds");
  }

  // Convert the angles to radians
  _theta1 = _theta1 * libMesh::pi / 180.0;
  _theta2 = _theta2 * libMesh::pi / 180.0;

  // Change the first angle to be measured from the +x axis
  _theta1 = 3.0 * libMesh::pi / 2.0 - _theta1;

  // Change the third angle to be measured from the +x axis
  _theta2 = _theta2 - libMesh::pi / 2.0;

  // Only compute the tangent once for computational efficiency
  _tan_theta1 = std::tan(_theta1);
  _tan_theta2 = std::tan(_theta2);
}

Real
TricrystalTripleJunctionIC::value(const Point & p)
{
  /*
   * This does all the work to create a triple junction that looks like the letter Y
   */
  Real dist_left;  // The distance from the point to the line specified by _theta1
  Real dist_right; // The distance from the point to the line specified by _theta2

  // Check if the point is above or below the left-most line
  // Function to use is y = _junction(1) + (x - _junction(0)) * std::tan(libMesh::pi/2.0 - _theta1)
  if (_theta1 == 0) // Handle tan(0) case
    dist_left = p(1) - _junction(1);
  else
    dist_left = p(1) - (_junction(1) + (p(0) - _junction(0)) * _tan_theta1);

  // Check if the point is above or below the right-most line
  // Function to use is y = _junction(1) + (x - _junction(0))*std::tan(-(libMesh::pi/2.0 - _theta2))
  if (_theta2 == 0) // Handle tan(0) classes
    dist_right = p(1) - _junction(1);
  else
    dist_right = p(1) - (_junction(1) + (p(0) - _junction(0)) * _tan_theta2);

  // Check if the point is to the left or right of the middle line
  Real dist_center = p(0) - _junction(0); // Negative value if the point is to the left

  if (_tan_theta1 > 0 && _theta1 <= libMesh::pi / 2.0) // Case for large left grain
  {
    /*
     * There's a lot going on here.  The first statement tells MOOSE to check and
     * see if the current point is above the line defined by the first angle, only
     * if it is past the center line.  All other points to the left of the center
     * line are going to be part of the 1st grain (_op_index == 1)
     * The second statement defines the second grain by the line defined by _theta2
     * and marks everything below that line, and to the right of the center line.
     * The third statement takes care of everything in between.
     */
    if ((((dist_left >= 0 && dist_center >= 0) || (dist_center < 0)) && _op_index == 1) ||
        (dist_right <= 0 && dist_center > 0 && _op_index == 2) ||
        (dist_left < 0 && dist_center > 0 && dist_right > 0 && _op_index == 3))
      return 1.0;
    else
      return 0.0;
  }

  // This does a similar thing as the above case, but switches it for the right and left grains
  else if (_tan_theta2 < 0 && _theta2 >= libMesh::pi / 2.0) // Case for large right grain
  {
    if ((dist_left <= 0 && dist_center <= 0 && _op_index == 1) ||
        (((dist_right >= 0 && dist_center <= 0) || (dist_center > 0)) && _op_index == 2) ||
        (dist_left > 0 && dist_right < 0 && dist_center < 0 && _op_index == 3))
      return 1.0;
    else
      return 0.0;
  }

  else // all other cases
  {
    if ((dist_left <= 0 && dist_center <= 0 && _op_index == 1) || // First grain
        (dist_right <= 0 && dist_center > 0 && _op_index == 2) || // Second grain
        (((dist_left > 0 && dist_center < 0) || (dist_right > 0 && dist_center >= 0)) &&
         _op_index == 3)) // Third grain
      return 1.0;
    else
      return 0.0;
  }
}
