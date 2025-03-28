//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"
#include "INSFVHydraulicSeparatorInterface.h"

class InputParameters;

/**
 * Class describing a hydraulic separator for the pressure in the
 * Navier Stokes equations. There is no cross flow and this should also
 * ensure that the cell gradients are decoupled on the two sides of the boundary.
 */
class INSFVPressureHydraulicSeparatorBC : public FVFluxBC, public INSFVHydraulicSeparatorInterface
{
public:
  static InputParameters validParams();
  INSFVPressureHydraulicSeparatorBC(const InputParameters & params);

  void computeResidual(const FaceInfo & /*fi*/) override {}
  void computeJacobian(const FaceInfo & /*fi*/) override {}
  void computeResidualAndJacobian(const FaceInfo & /*fi*/) override {}

  ADReal computeQpResidual() override final
  {
    mooseError("Pressure hydraulic separators are not supposed to contribute to anything!");
  }
};
