//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSMomentumBase.h"

// Forward Declarations

/**
 * This class computes momentum equation residual and Jacobian viscous
 * contributions for the "Laplacian" form of the governing equations.
 */
class INSMomentumLaplaceForm : public INSMomentumBase
{
public:
  static InputParameters validParams();

  INSMomentumLaplaceForm(const InputParameters & parameters);

  virtual ~INSMomentumLaplaceForm() {}

protected:
  virtual Real computeQpResidualViscousPart() override;
  virtual Real computeQpJacobianViscousPart() override;
  virtual Real computeQpOffDiagJacobianViscousPart(unsigned jvar) override;
};
