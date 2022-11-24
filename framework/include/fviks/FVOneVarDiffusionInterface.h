//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInterfaceKernel.h"

class FVOneVarDiffusionInterface : public FVInterfaceKernel
{
public:
  static InputParameters validParams();
  FVOneVarDiffusionInterface(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  const Moose::Functor<ADReal> & _coeff1;
  const Moose::Functor<ADReal> & _coeff2;

  /// Decides if a geometric arithmetic or harmonic average is used for the
  /// face interpolation of the diffusion coefficient.
  Moose::FV::InterpMethod _coeff_interp_method;
};
