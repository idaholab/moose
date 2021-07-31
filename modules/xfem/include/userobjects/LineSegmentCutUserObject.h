//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometricCut2DUserObject.h"

// Forward declarations

class LineSegmentCutUserObject : public GeometricCut2DUserObject
{
public:
  static InputParameters validParams();

  LineSegmentCutUserObject(const InputParameters & parameters);

  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;

  virtual const std::vector<RealVectorValue>
  getCrackPlaneNormals(unsigned int num_crack_front_points) const override;

protected:
  std::vector<Real> _cut_data;
};
