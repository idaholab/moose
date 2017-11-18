/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LINESEGMENTCUTSETUSEROBJECT_H
#define LINESEGMENTCUTSETUSEROBJECT_H

#include "GeometricCut2DUserObject.h"

// Forward declarations
class LineSegmentCutSetUserObject;

template <>
InputParameters validParams<LineSegmentCutSetUserObject>();

class LineSegmentCutSetUserObject : public GeometricCut2DUserObject
{
public:
  LineSegmentCutSetUserObject(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};
  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;

protected:
  std::vector<Real> _cut_data;
};

#endif // LINESEGMENTCUTSETUSEROBJECT_H
