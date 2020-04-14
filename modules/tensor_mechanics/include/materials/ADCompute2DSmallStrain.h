//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeSmallStrain.h"

/**
 * ADCompute2DSmallStrain defines a strain tensor, assuming small strains,
 * in 2D geometries / simulations.  ComputePlaneSmallStrain acts as a
 * base class for ComputePlaneSmallStrain and ComputeAxisymmetricRZSmallStrain
 * through the computeOutOfPlaneStrain method.
 */
class ADCompute2DSmallStrain : public ADComputeSmallStrain
{
public:
  static InputParameters validParams();

  ADCompute2DSmallStrain(const InputParameters & parameters);

  void initialSetup() override;
  virtual void computeProperties() override;

protected:
  virtual void displacementIntegrityCheck() override;
  virtual ADReal computeOutOfPlaneStrain() = 0;

  const unsigned int _out_of_plane_direction;
};
