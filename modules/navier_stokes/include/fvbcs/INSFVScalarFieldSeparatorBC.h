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
 * Class describing a separator (no diffusive or advective flux) for a scalar field
 * (pressure, energy, passive scalar) associated with the
 * Navier Stokes equations.
 */
class INSFVScalarFieldSeparatorBC : public FVFluxBC, public INSFVHydraulicSeparatorInterface
{
public:
  static InputParameters validParams();
  INSFVScalarFieldSeparatorBC(const InputParameters & params);

  void computeResidual(const FaceInfo & /*fi*/) override {}
  void computeJacobian(const FaceInfo & /*fi*/) override {}
  void computeResidualAndJacobian(const FaceInfo & /*fi*/) override {}

  ADReal computeQpResidual() override final
  {
    mooseError("Scalar field separators are not supposed to contribute to anything!");
  }
};
