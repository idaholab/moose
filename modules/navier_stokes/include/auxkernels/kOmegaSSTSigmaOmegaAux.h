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

/*
 *Computes the mixing length for the mixing length turbulence model.
 */
class kOmegaSSTSigmaOmegaAux : public AuxKernel
{
public:
  static InputParameters validParams();

  kOmegaSSTSigmaOmegaAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// F1 blending function
  const Moose::Functor<ADReal> & _F1;

  /// C-mu closure coefficient
  static constexpr Real _sigma_omega_1 = 0.5; //2.000;
  static constexpr Real _sigma_omega_2 = 0.856; //1.168;
};
