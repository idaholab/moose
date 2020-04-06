//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowDiffusivityBase.h"

/// Material designed to provide constant tortuosity and diffusion coefficents
class PorousFlowDiffusivityConst : public PorousFlowDiffusivityBase
{
public:
  static InputParameters validParams();

  PorousFlowDiffusivityConst(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Input tortuosity
  const std::vector<Real> _input_tortuosity;
};
