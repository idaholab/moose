//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * Adds grad-div stabilization to the INS momentum equation
 */
class INSADMomentumGradDiv : public ADVectorKernel
{
public:
  static InputParameters validParams();

  INSADMomentumGradDiv(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// The grad-div stabilization coefficient
  const Real _gamma;

  /// Coordinate system of the mesh
  const Moose::CoordinateSystemType & _coord_sys;

  /// The radial coordinate index for RZ coordinate systems
  const unsigned int _rz_radial_coord;
};
