#include "RZSymmetry.h"
#include "libmesh/point.h"

template <>
InputParameters
validParams<RZSymmetry>()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<Point>("axis_point", "A point on the axis of RZ symmetry.");
  params.addRequiredParam<RealVectorValue>("axis_dir", "The direction of the axis of RZ symmetry.");
  return params;
}

RZSymmetry::RZSymmetry(const InputParameters & parameters)
  : _axis_point(0., 0., 0.), _axis_dir(parameters.get<RealVectorValue>("axis_dir"))
{
  const Point pt = parameters.get<Point>("axis_point");
  _axis_point = Point(pt(0), pt(1), pt(2));
}

Real
RZSymmetry::computeCircumference(const RealVectorValue & pt)
{
  RealVectorValue v = (pt - _axis_point);
  const Real r = v.cross(_axis_dir).norm() / _axis_dir.norm();
  return 2 * libMesh::pi * r;
}
