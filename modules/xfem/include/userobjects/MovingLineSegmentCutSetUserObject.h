//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOVINGMovingLineSegmentCutSetUserObject_H
#define MOVINGMovingLineSegmentCutSetUserObject_H

#include "LineSegmentCutSetUserObject.h"
#include "XFEMMovingInterfaceVelocityBase.h"

// Forward declarations
class MovingLineSegmentCutSetUserObject;
class PointValueAtXFEMInterface;

template <>
InputParameters validParams<MovingLineSegmentCutSetUserObject>();

class MovingLineSegmentCutSetUserObject : public LineSegmentCutSetUserObject

{
public:
  MovingLineSegmentCutSetUserObject(const InputParameters & parameters);

  virtual void initialize() override;

  virtual void execute() override;

  virtual void finalize() override;

  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;

  virtual Real cutFraction(unsigned int cut_num, Real time) const override;

  /// Pointer to XFEMMovingInterfaceVelocityBase object
  const XFEMMovingInterfaceVelocityBase * _interface_velocity;
};

#endif // MOVINGMovingLineSegmentCutSetUserObject_H
