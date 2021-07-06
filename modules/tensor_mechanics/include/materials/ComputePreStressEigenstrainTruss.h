//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainTrussBase.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputePreStressEigenstrainTruss is a base class for all models that
 * compute beam eigenstrains due to thermal expansion of a material.
 */
class ComputePreStressEigenstrainTruss
  : public DerivativeMaterialInterface<ComputeEigenstrainTrussBase>
{
public:
  static InputParameters validParams();

  ComputePreStressEigenstrainTruss(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;
  const Real _pre_stressing_strain;
};
