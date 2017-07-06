/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LINE_SEGMENT_CUT_USER_OBJECT_H
#define LINE_SEGMENT_CUT_USER_OBJECT_H

#include "GeometricCut2DUserObject.h"

// Forward declarations
class LineSegmentCutUserObject;

template <>
InputParameters validParams<LineSegmentCutUserObject>();

class LineSegmentCutUserObject : public GeometricCut2DUserObject
{
public:
  LineSegmentCutUserObject(const InputParameters & parameters);
  ~LineSegmentCutUserObject();

  virtual void initialize(){};
  virtual void execute(){};
  virtual void finalize(){};

protected:
  std::vector<Real> _cut_data;
};

#endif // LINE_SEGMENT_CUT_USER_OBJECT_H
