/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RECTANGLE_CUT_USEROBJECT_H
#define RECTANGLE_CUT_USEROBJECT_H

#include "GeometricCut3DUserObject.h"

// Forward declarations
class RectangleCutUserObject;

template <>
InputParameters validParams<RectangleCutUserObject>();

class RectangleCutUserObject : public GeometricCut3DUserObject
{
public:
  RectangleCutUserObject(const InputParameters & parameters);
  ~RectangleCutUserObject();

  virtual void initialize(){};
  virtual void execute(){};
  virtual void finalize(){};

protected:
  std::vector<Real> _cut_data;

private:
  std::vector<Point> _vertices;

  bool isInsideCutPlane(Point p) const;
};

#endif // RECTANGLE_CUT_USEROBJECT_H
