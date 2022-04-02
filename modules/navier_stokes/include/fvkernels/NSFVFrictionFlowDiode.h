//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVElementalKernel.h"

/**
 * This kernels adds a linear friction penalty if the velocity is in the halfplane opposite the
 * normal of the diode.
 */
class NSFVFrictionFlowDiode : public INSFVElementalKernel
{
public:
  static InputParameters validParams();

  NSFVFrictionFlowDiode(const InputParameters & parameters);

  using INSFVElementalKernel::gatherRCData;
  void gatherRCData(const Elem &) override;

private:
  /// Direction of the diode
  const RealVectorValue _direction;

  /// A linear friction coefficient applied when velocity is opposite the direction
  const Real _resistance;
};
