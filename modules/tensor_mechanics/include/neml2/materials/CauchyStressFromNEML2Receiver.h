//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CauchyStressFromNEML2UO.h"
#include "ComputeLagrangianObjectiveStress.h"

/**
 * This is a "glue" material that retrieves the batched output vector from a NEML2 material model
 * and uses the output variables to perform the objective stress integration.
 */
class CauchyStressFromNEML2Receiver : public ComputeLagrangianObjectiveStress
{
public:
  static InputParameters validParams();
  CauchyStressFromNEML2Receiver(const InputParameters & parameters);

#ifndef NEML2_ENABLED

protected:
  virtual void computeQpSmallStress() override {}

#else

protected:
  virtual void computeQpSmallStress() override;

  /// The NEML2 user object that actually performs the batched computation
  const CauchyStressFromNEML2UO & _neml2_uo;

  /// The output from the NEML2 user object
  const CauchyStressFromNEML2UO::OutputVector & _output;

#endif // NEML2_ENABLED
};
