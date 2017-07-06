/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "EllipseCutUserObject.h"

// MOOSE includes
#include "MooseError.h"

// XFEM includes
#include "XFEMFuncs.h"
//#include "EFAFuncs.h"

template <>
InputParameters
validParams<EllipseCutUserObject>()
{
  // Get input parameters from parent class
  InputParameters params = validParams<GeometricCut3DUserObject>();

  // Add required parameters
  params.addRequiredParam<std::vector<Real>>("cut_data",
                                             "Vector of Real values providing cut information");
  // Class description
  params.addClassDescription("Creates a UserObject for elliptical cuts on 3D meshes for XFEM");
  // Return the parameters
  return params;
}

EllipseCutUserObject::EllipseCutUserObject(const InputParameters & parameters)
  : GeometricCut3DUserObject(parameters), _cut_data(getParam<std::vector<Real>>("cut_data"))
{
  // Throw error if length of cut_data is incorrect
  if (_cut_data.size() != 9)
    mooseError("Length of EllipseCutUserObject cut_data must be 9");

  // Assign cut_data to vars used to construct cuts
  _center = Point(_cut_data[0], _cut_data[1], _cut_data[2]);
  _vertices.push_back(Point(_cut_data[3], _cut_data[4], _cut_data[5]));
  _vertices.push_back(Point(_cut_data[6], _cut_data[7], _cut_data[8]));

  Point ray1 = _vertices[0] - _center;
  Point ray2 = _vertices[1] - _center;

  if (std::abs(ray1 * ray2) > 1e-6)
    mooseError(
        "EllipseCutUserObject only works on an elliptic cut. Users should provide two points at "
        "the long and short axis.");

  _normal = ray1.cross(ray2);
  Xfem::normalizePoint(_normal);

  Real R1 = std::sqrt(ray1.norm_sq());
  Real R2 = std::sqrt(ray2.norm_sq());

  // Determine which the long and short axes
  if (R1 > R2)
  {
    _unit_vec1 = ray1;
    _unit_vec2 = ray2;
    _long_axis = R1;
    _short_axis = R2;
  }
  else
  {
    _unit_vec1 = ray2;
    _unit_vec2 = ray1;
    _long_axis = R2;
    _short_axis = R1;
  }

  Xfem::normalizePoint(_unit_vec1);
  Xfem::normalizePoint(_unit_vec2);
}

EllipseCutUserObject::~EllipseCutUserObject() {}

bool
EllipseCutUserObject::isInsideCutPlane(Point p) const
{
  Point ray = p - _center;
  if (std::abs(ray * _normal) < 1e-6)
  {
    double xloc = ray * _unit_vec1;
    double yloc = ray * _unit_vec2;

    if (std::sqrt(xloc * xloc / (_long_axis * _long_axis) +
                  yloc * yloc / (_short_axis * _short_axis)) < 1)
      return true;
  }
  return false;
}
