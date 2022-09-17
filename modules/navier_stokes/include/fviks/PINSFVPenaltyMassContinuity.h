//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInterfaceKernel.h"

class PINSFVPenaltyMassContinuity : public FVInterfaceKernel
{
public:
  static InputParameters validParams();
  PINSFVPenaltyMassContinuity(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  const Real _penalty;
  const MooseVariableFV<Real> & _u1;
  const MooseVariableFV<Real> & _u2;
};
