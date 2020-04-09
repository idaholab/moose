//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSMomentumTractionForm.h"

// Forward Declarations

/**
 * This class computes additional momentum equation residual and
 * Jacobian contributions for the incompressible Navier-Stokes
 * momentum equation in RZ (axisymmetric cylindrical) coordinates.
 */
class INSMomentumTractionFormRZ : public INSMomentumTractionForm
{
public:
  static InputParameters validParams();

  INSMomentumTractionFormRZ(const InputParameters & parameters);

  virtual ~INSMomentumTractionFormRZ() {}

protected:
  virtual RealVectorValue strongViscousTermTraction() override;
  virtual RealVectorValue dStrongViscDUCompTraction(unsigned comp) override;
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned jvar) override;
};
