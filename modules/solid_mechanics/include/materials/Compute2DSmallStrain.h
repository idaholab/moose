//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeSmallStrain.h"

/**
 * Compute2DSmallStrain defines a strain tensor, assuming small strains,
 * in 2D geometries / simulations.  ComputePlaneSmallStrain acts as a
 * base class for ComputePlaneSmallStrain and ComputeAxisymmetricRZSmallStrain
 * through the computeOutOfPlaneStrain method.
 */
class Compute2DSmallStrain : public ComputeSmallStrain
{
public:
  static InputParameters validParams();

  Compute2DSmallStrain(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void computeProperties() override;
  virtual void displacementIntegrityCheck() override;
  virtual Real computeOutOfPlaneStrain() = 0;

  const unsigned int _out_of_plane_direction;
};
