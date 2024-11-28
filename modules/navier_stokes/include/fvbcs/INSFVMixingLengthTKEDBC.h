//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVDirichletBCBase.h"

/**
 * A class for turbulent kinetic energy dissipation rate inlet boundary conditions
 */
class INSFVMixingLengthTKEDBC : public FVDirichletBCBase
{
public:
  static InputParameters validParams();
  INSFVMixingLengthTKEDBC(const InputParameters & params);
  ADReal boundaryValue(const FaceInfo & fi, const Moose::StateArg & state) const override;

protected:
  /// C-mu closure coefficient
  const Moose::Functor<ADReal> & _C_mu;

  /// turbulent kinetic energy
  const Moose::Functor<ADReal> & _k;

  /// turbulent intensity
  const Moose::Functor<ADReal> & _characteristic_length;
};
