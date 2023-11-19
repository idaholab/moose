//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshDivision.h"

class Positions;

/**
 * Divides the mesh based on a nearest-neighbor algorithm applied to the positions
 * from a Positions object
 */
class NearestPositionsDivision : public MeshDivision
{
public:
  static InputParameters validParams();

  NearestPositionsDivision(const InputParameters & parameters);

  virtual void initialize() override;
  virtual unsigned int divisionIndex(const Point & pt) const override;
  virtual unsigned int divisionIndex(const Elem & elem) const override;

private:
  // Positions object to use to create the nearest-neighbor division/partition
  const Positions * const _nearest_positions_obj;
};
