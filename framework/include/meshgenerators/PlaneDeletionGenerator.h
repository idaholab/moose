//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementDeletionGeneratorBase.h"

namespace libmesh
{
class Point;
}

/**
 * Deletes elements lying above a plane
 */
class PlaneDeletionGenerator : public ElementDeletionGeneratorBase
{
public:
  static InputParameters validParams();

  PlaneDeletionGenerator(const InputParameters & parameters);

protected:
  virtual bool shouldDelete(const Elem * elem) override;

private:
  /// Point defining the plane
  const Point & _point;

  /// Normal vector
  RealVectorValue _normal;
};
