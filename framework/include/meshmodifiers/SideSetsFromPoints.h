//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SIDESETSFROMPOINTS_H
#define SIDESETSFROMPOINTS_H

#include "AddSideSetsBase.h"

class SideSetsFromPoints;

template <>
InputParameters validParams<SideSetsFromPoints>();

class SideSetsFromPoints : public AddSideSetsBase
{
public:
  SideSetsFromPoints(const InputParameters & parameters);

protected:
  virtual void modify() override;

  std::vector<BoundaryName> _boundary_names;

  std::vector<Point> _points;
};

#endif /* SIDESETSFROMPOINTS_H */
