//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMomentumGravity.h"

/**
 * Imposes a gravitational force on the momentum equation in porous media in Rhie-Chow
 * (incompressible) contexts
 */
class PINSFVMomentumGravity : public INSFVMomentumGravity
{
public:
  static InputParameters validParams();
  PINSFVMomentumGravity(const InputParameters & params);

  using INSFVMomentumGravity::gatherRCData;
  void gatherRCData(const Elem &) override;

protected:
  /// the porosity
  const Moose::Functor<ADReal> & _eps;
};
