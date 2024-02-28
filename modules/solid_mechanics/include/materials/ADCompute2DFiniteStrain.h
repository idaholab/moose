//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeFiniteStrain.h"

/**
 * ADCompute2DFiniteStrain defines a strain increment and a rotation increment
 * for finite strains in 2D geometries, handling the out of plane strains.
 * ADCompute2DFiniteStrain contains a virtual method to define the out-of-plane strain
 * as a general nonzero value in the inherited classes ComputePlaneFiniteStrain
 * and ComputeAxisymmetricRZFiniteStrain.
 */
class ADCompute2DFiniteStrain : public ADComputeFiniteStrain
{
public:
  static InputParameters validParams();

  ADCompute2DFiniteStrain(const InputParameters & parameters);

  void initialSetup() override;

  virtual void computeProperties() override;

protected:
  virtual void displacementIntegrityCheck() override;

  /**
   * Computes the current out-of-plane component of the displacement gradient; as a virtual
   *function, this function is overwritten for the specific geometries defined by inheriting classes
   */
  virtual ADReal computeOutOfPlaneGradDisp() = 0;

  /**
   * Computes the old out-of-plane component of the displacement gradient; as a virtual function,
   * this function is overwritten for the specific geometries defined by inheriting classes
   */
  virtual Real computeOutOfPlaneGradDispOld() = 0;

  const unsigned int _out_of_plane_direction;
};
