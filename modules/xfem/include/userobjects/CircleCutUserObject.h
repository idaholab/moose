/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CIRCLE_CUT_USEROBJECT_H
#define CIRCLE_CUT_USEROBJECT_H

#include "GeometricCut3DUserObject.h"

// Forward declarations
class CircleCutUserObject;

template <>
InputParameters validParams<CircleCutUserObject>();

class CircleCutUserObject : public GeometricCut3DUserObject
{
public:
  CircleCutUserObject(const InputParameters & parameters);
  ~CircleCutUserObject();

  virtual void initialize(){};
  virtual void execute(){};
  virtual void finalize(){};

protected:
  std::vector<Real> _cut_data;

private:
  std::vector<Point> _vertices;
  Real _radius;
  Real _angle;

  virtual bool isInsideCutPlane(Point p) const;
};

#endif // CIRCLE_CUT_USEROBJECT_H
