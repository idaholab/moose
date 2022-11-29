//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInterfaceKernel.h"

class FVOnlyAddDiffusionToOneSideOfInterface : public FVInterfaceKernel
{
public:
  static InputParameters validParams();
  FVOnlyAddDiffusionToOneSideOfInterface(const InputParameters & params);

  void computeResidual(const FaceInfo & fi) override;
  void computeJacobian(const FaceInfo & fi) override;
  void computeResidualAndJacobian(const FaceInfo & fi) override;

protected:
  ADReal computeQpResidual() override;

  const Moose::Functor<ADReal> & _coeff2;
};
