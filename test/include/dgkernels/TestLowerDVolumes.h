//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HFEMDiffusion.h"

class TestLowerDVolumes : public HFEMDiffusion
{
public:
  static InputParameters validParams();

  TestLowerDVolumes(const InputParameters & parameters);
  void computeResidual() override;

protected:
  const Real & _lower_d_vol;
  const Real _h;
  const Real _area;
};
