//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"
#include "INSFVMomentumResidualObject.h"

class INSFVMomentumPressure : public FVElementalKernel, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  INSFVMomentumPressure(const InputParameters & params);

  // This object neither contributes to the A coefficients nor to the B (source) coefficients
  void gatherRCData(const Elem &) override {}
  void gatherRCData(const FaceInfo &) override {}

protected:
  ADReal computeQpResidual() override;

  /// The pressure
  const Moose::Functor<ADReal> & _p;

  /// index x|y|z
  const unsigned int _index;

  /// Whether to correct for mesh skewness in face calculations
  const bool _correct_skewness;
};
