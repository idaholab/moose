//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "RankTwoTensor.h"

/**
 * CylindricalRankTwoAux is designed to take the data in the CylindricalRankTwoTensor material
 * property, for example stress or strain, and output the value for the
 * supplied indices in cylindrical coordinates, where the cylindrical plane axis is
 * along the z-axis and the center point in the x-y plan is defined by by center_point.
 */

class CylindricalRankTwoAux : public AuxKernel
{
public:
  static InputParameters validParams();

  CylindricalRankTwoAux(const InputParameters & parameters);
  virtual ~CylindricalRankTwoAux() {}

protected:
  virtual Real computeValue();
  const MaterialProperty<RankTwoTensor> & _tensor;
  const unsigned int _i;
  const unsigned int _j;
  const Point _center_point;
};
