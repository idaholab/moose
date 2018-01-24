//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
