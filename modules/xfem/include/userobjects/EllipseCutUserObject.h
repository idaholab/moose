/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef ELLIPSECUTUSEROBJECT_H
#define ELLIPSECUTUSEROBJECT_H

#include "GeometricCut3DUserObject.h"

// Forward declarations
class EllipseCutUserObject;

template <>
InputParameters validParams<EllipseCutUserObject>();

class EllipseCutUserObject : public GeometricCut3DUserObject
{
public:
  EllipseCutUserObject(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};
  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;

protected:
  std::vector<Real> _cut_data;

private:
  std::vector<Point> _vertices;
  Point _unit_vec1;
  Point _unit_vec2;
  Real _long_axis;
  Real _short_axis;

  virtual bool isInsideCutPlane(Point p) const override;
};

#endif // ELLIPSECUTUSEROBJECT_H
