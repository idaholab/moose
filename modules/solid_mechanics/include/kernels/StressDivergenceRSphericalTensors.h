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
 * StressDivergenceRSphericalTensors is a modification of StressDivergenceTensors
 * for 1D spherically symmetric problems.  The main modifications from the original
 * StressDivergenceTensors code are requirements from the dependence of stress in
 * the polar and azimuthal stresses on displacement and position in the radial
 * direction.  This kernel is for symmetrical loading only.
 * If solving an anisotropic material problem, recall that the orientation of
 * the basis vectors (\hat{e}_r) change with position, so the components of the
 * elasticity tensor are functions of position.
 * Reference: Bower, A.F. Applied Mechanics of Solids (2012). Chapter 4. Available
 * online at solidmechanics.org
 * Within this kernel, '_disp_x' refers to displacement in the radial direction.
 * The COORD_TYPE in the Problem block must be set to RSpherical.
 */
class StressDivergenceRSphericalTensors : public StressDivergenceTensors
{
public:
  static InputParameters validParams();

  StressDivergenceRSphericalTensors(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  Real calculateJacobian(unsigned int ivar, unsigned int jvar);
};
