/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LINESEGMENTCUTUSEROBJECT_H
#define LINESEGMENTCUTUSEROBJECT_H

#include "GeometricCut2DUserObject.h"

// Forward declarations
class LineSegmentCutUserObject;

template <>
InputParameters validParams<LineSegmentCutUserObject>();

class LineSegmentCutUserObject : public GeometricCut2DUserObject
{
public:
  LineSegmentCutUserObject(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};
  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;

protected:
  std::vector<Real> _cut_data;
};

#endif // LINESEGMENTCUTUSEROBJECT_H
