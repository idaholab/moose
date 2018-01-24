//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SIDESETSFROMNORMALS_H
#define SIDESETSFROMNORMALS_H

#include "AddSideSetsBase.h"

class SideSetsFromNormals;

template <>
InputParameters validParams<SideSetsFromNormals>();

class SideSetsFromNormals : public AddSideSetsBase
{
public:
  SideSetsFromNormals(const InputParameters & parameters);

protected:
  virtual void modify() override;

  std::vector<BoundaryName> _boundary_names;

  std::vector<Point> _normals;
};

#endif /* SIDESETSFROMNORMALS_H */
