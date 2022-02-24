//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVTimeKernel.h"

class INSFVMomentumTimeDerivative : public INSFVTimeKernel
{
public:
  static InputParameters validParams();
  INSFVMomentumTimeDerivative(const InputParameters & params);

  using INSFVTimeKernel::gatherRCData;
  void gatherRCData(const Elem &) override;

protected:
  /// The density
  const Moose::Functor<ADReal> & _rho;
};
