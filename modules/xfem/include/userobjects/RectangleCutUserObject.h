/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RECTANGLECUTUSEROBJECT_H
#define RECTANGLECUTUSEROBJECT_H

#include "GeometricCut3DUserObject.h"

// Forward declarations
class RectangleCutUserObject;

template <>
InputParameters validParams<RectangleCutUserObject>();

class RectangleCutUserObject : public GeometricCut3DUserObject
{
public:
  RectangleCutUserObject(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};

protected:
  Point _vertex_point1;
  Point _vertex_point2;
  Point _vertex_point3;
  Point _vertex_point4;

private:
  std::vector<Point> _vertices;

  bool isInsideCutPlane(Point p) const override;
};

#endif // RECTANGLECUTUSEROBJECT_H
