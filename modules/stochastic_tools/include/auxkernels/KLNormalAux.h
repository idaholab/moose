//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class KLExpansionUserObject;

/**
 * Samples the correlated Gaussian field of a KLExpansionUserObject, shifted by a constant mean
 */
class KLNormalAux : public AuxKernel
{
public:
  static InputParameters validParams();
  KLNormalAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const KLExpansionUserObject & _kl_uo;
  /// Constant added to the sampled field
  const Real _mean;
};
