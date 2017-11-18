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
  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;

protected:
  std::vector<Real> _cut_data;

private:
  std::vector<Point> _vertices;

  bool isInsideCutPlane(Point p) const override;
};

#endif // RECTANGLECUTUSEROBJECT_H
