//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInterfaceKernel.h"

class PINSFVPenaltyBernoulli : public FVInterfaceKernel
{
public:
  static InputParameters validParams();
  PINSFVPenaltyBernoulli(const InputParameters & params);

  void computeResidual(const FaceInfo & fi) override;
  void computeJacobian(const FaceInfo & fi) override;

protected:
  ADReal computeQpResidual() override;

  const Real _penalty;
  const MooseVariableFV<Real> & _v1;
  const MooseVariableFV<Real> & _v2;
  const MooseVariableFV<Real> & _p1;
  const MooseVariableFV<Real> & _p2;
  const MooseVariableFV<Real> & _eps1;
  const MooseVariableFV<Real> & _eps2;
  const Real _rho;
};
