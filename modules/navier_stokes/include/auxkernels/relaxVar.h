//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
/**
 * Computes wall y+ based on wall functions.
 */
class relaxVar : public AuxKernel
{
public:
  static InputParameters validParams();

  relaxVar(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Turbulent kinetic energy
  const Moose::Functor<ADReal> & _var_to_relax;
  const Real _relaxation_factor;
};