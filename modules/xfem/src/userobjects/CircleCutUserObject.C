/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CircleCutUserObject.h"

// MOOSE includes
#include "MooseError.h"

// XFEM includes
#include "XFEMFuncs.h"
//#include "EFAFuncs.h"

template <>
InputParameters
validParams<CircleCutUserObject>()
{
  // Get input parameters from parent class
  InputParameters params = validParams<GeometricCut3DUserObject>();

  // Add required parameters
  params.addRequiredParam<std::vector<Real>>("cut_data",
                                             "Vector of Real values providing cut information");
  // Class description
  params.addClassDescription("Creates a UserObject for circular cuts on 3D meshes for XFEM");
  // Return the parameters
  return params;
}

CircleCutUserObject::CircleCutUserObject(const InputParameters & parameters)
  : GeometricCut3DUserObject(parameters), _cut_data(getParam<std::vector<Real>>("cut_data"))
{
  // Throw error if length of cut_data is incorrect
  if (_cut_data.size() != 9)
    mooseError("Length of CircleCutUserObject cut_data must be 9");

  // Assign cut_data to vars used to construct cuts
  _center = Point(_cut_data[0], _cut_data[1], _cut_data[2]);
  _vertices.push_back(Point(_cut_data[3], _cut_data[4], _cut_data[5]));
  _vertices.push_back(Point(_cut_data[6], _cut_data[7], _cut_data[8]));

  Point ray1 = _vertices[0] - _center;
  Point ray2 = _vertices[1] - _center;

  _normal = ray1.cross(ray2);
  Xfem::normalizePoint(_normal);

  Real R1 = std::sqrt(ray1.norm_sq());
  Real R2 = std::sqrt(ray2.norm_sq());
  if (std::abs(R1 - R2) > 1e-10)
    mooseError("CircleCutUserObject only works for a circular cut");

  _radius = 0.5 * (R1 + R2);
  _angle = std::acos((ray1 * ray2) / (R1 * R2));
}

CircleCutUserObject::~CircleCutUserObject() {}

bool
CircleCutUserObject::isInsideCutPlane(Point p) const
{
  Point ray = p - _center;
  if (std::abs(ray * _normal) < 1e-15 && std::sqrt(ray.norm_sq()) < _radius)
    return true;
  return false;
}
