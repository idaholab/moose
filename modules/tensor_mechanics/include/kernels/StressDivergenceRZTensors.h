//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StressDivergenceTensors.h"

// Forward Declarations

/**
 * StressDivergenceRZTensors is a modification of StressDivergenceTensors to
 * accommodate the Axisymmetric material models that use cylindrical coordinates.
 * This kernel is for symmetrical loading only.  The key modifications are a result
 * of the circumferential stress' dependence on displacement in the axial direction.
 * Reference: Cook et.al. Concepts and Applications of Finite Element Analysis,
 * 4th Ed. 2002. p 510.
 * Within this kernel, '_disp_x' refers to displacement in the radial direction,
 * u_r, and '_disp_y' refers to displacement in the axial direction, u_z.
 * The COORD_TYPE in the Problem block must be set to RZ.
 */
class StressDivergenceRZTensors : public StressDivergenceTensors
{
public:
  static InputParameters validParams();

  StressDivergenceRZTensors(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  virtual void computeAverageGradientTest() override;
  virtual void computeAverageGradientPhi() override;

  Real calculateJacobian(unsigned int ivar, unsigned int jvar);
};
