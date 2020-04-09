//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralizedPlaneStrainUserObjectBasePD.h"

/**
 * UserObject class to compute the residual and diagonal Jacobian components for scalar out-of-plane
 * strain variable of generalized plane strain model based on ordinary state-based peridynamic
 * model
 */
class GeneralizedPlaneStrainUserObjectOSPD : public GeneralizedPlaneStrainUserObjectBasePD
{
public:
  static InputParameters validParams();

  GeneralizedPlaneStrainUserObjectOSPD(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Variable for out-of-plane stress component
  MooseVariable * _out_of_plane_stress_var;
};
