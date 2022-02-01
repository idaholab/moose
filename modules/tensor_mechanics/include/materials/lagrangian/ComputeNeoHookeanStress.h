//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressPK2.h"

/// Compressible Neo-Hookean hyperelasticity
///
///  Model follows from W = lambda / 2 * (ln J)^2 - mu * ln J + 1/2 * mu *
///  (tr(C)- I)
///
///  with C = 1/2*(F.T*F-I) and J = det(F)
///
class ComputeNeoHookeanStress : public ComputeLagrangianStressPK2
{
public:
  static InputParameters validParams();
  ComputeNeoHookeanStress(const InputParameters & parameters);

protected:
  /// Actual stress/Jacobian update
  virtual void computeQpPK2Stress();

protected:
  const MaterialProperty<Real> & _lambda;
  const MaterialProperty<Real> & _mu;
};
