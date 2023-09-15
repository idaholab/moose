//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVAdvectionKernel.h"

/**
 * A flux kernel transporting mass across cell faces
 */
class INSFVMassAdvection : public INSFVAdvectionKernel
{
public:
  static InputParameters validParams();
  INSFVMassAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;
  virtual bool hasMaterialTimeDerivative() const { return false; }

  /// Density
  const Moose::Functor<ADReal> & _rho;
};
