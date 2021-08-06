//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMomentumBoussinesq.h"

/**
 * Imposes a Boussinesq force on the momentum equation. Useful for modeling natural convection
 * within an incompressible Navier-Stokes approximation in porous media
 */
class PINSFVMomentumBoussinesq : public INSFVMomentumBoussinesq
{
public:
  static InputParameters validParams();
  PINSFVMomentumBoussinesq(const InputParameters & params);

  using INSFVMomentumBoussinesq::gatherRCData;
  void gatherRCData(const Elem & elem) override;

protected:
  /// the porosity
  const Moose::Functor<ADReal> & _eps;
};
