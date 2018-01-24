//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PRIMARYTIMEDERIVATIVE
#define PRIMARYTIMEDERIVATIVE

#include "TimeDerivative.h"

// Forward Declaration
class PrimaryTimeDerivative;

template <>
InputParameters validParams<PrimaryTimeDerivative>();

/**
 * Define the Kernel for a CoupledConvectionReactionSub operator that looks like:
 * storage * delta pressure / delta t
 */
class PrimaryTimeDerivative : public TimeDerivative
{
public:
  PrimaryTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Material property of porosity
  const MaterialProperty<Real> & _porosity;
};

#endif // PRIMARYTIMEDERIVATIVE
