//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSBase.h"

// Forward Declarations

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
class INSMomentumBase : public INSBase
{
public:
  static InputParameters validParams();

  INSMomentumBase(const InputParameters & parameters);

  virtual ~INSMomentumBase() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
  virtual Real computeQpResidualViscousPart() = 0;
  virtual Real computeQpJacobianViscousPart() = 0;
  virtual Real computeQpOffDiagJacobianViscousPart(unsigned jvar) = 0;

  virtual Real computeQpPGResidual();
  virtual Real computeQpPGJacobian(unsigned comp);

  unsigned _component;
  bool _integrate_p_by_parts;
  bool _supg;
  const Function & _ffn;
};
