//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseVariableFV.h"
#include "INSFVMomentumAdvection.h"

/**
 * A flux kernel transporting momentum in porous media across cell faces
 */
class PINSFVMomentumAdvection : public INSFVMomentumAdvection
{
public:
  static InputParameters validParams();
  PINSFVMomentumAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// porosity functor
  const Moose::Functor<ADReal> & _eps;
};
