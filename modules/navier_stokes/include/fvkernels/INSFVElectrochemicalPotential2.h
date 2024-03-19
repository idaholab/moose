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

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics and that advects arbitrary scalar quantities
 */
class INSFVElectrochemicalPotential2 : public FVElementalKernel
{
public:
  static InputParameters validParams();
  INSFVElectrochemicalPotential2(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

    const std::vector<int> & _z;
    const Moose::Functor<ADReal> & _epsilonr;
    std::vector<const Moose::Functor<ADReal> *> _c;
};
