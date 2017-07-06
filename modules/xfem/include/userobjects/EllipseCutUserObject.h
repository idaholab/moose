/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef ELLIPSE_CUT_USEROBJECT_H
#define ELLIPSE_CUT_USEROBJECT_H

#include "GeometricCut3DUserObject.h"

// Forward declarations
class EllipseCutUserObject;

template <>
InputParameters validParams<EllipseCutUserObject>();

class EllipseCutUserObject : public GeometricCut3DUserObject
{
public:
  EllipseCutUserObject(const InputParameters & parameters);
  ~EllipseCutUserObject();

  virtual void initialize(){};
  virtual void execute(){};
  virtual void finalize(){};

protected:
  std::vector<Real> _cut_data;

private:
  std::vector<Point> _vertices;
  Point _unit_vec1;
  Point _unit_vec2;
  Real _long_axis;
  Real _short_axis;

  virtual bool isInsideCutPlane(Point p) const;
};

#endif // ELLIPSE_CUT_USEROBJECT_H
