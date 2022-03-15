//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PenaltyDirichletBC.h"

class PenaltyLimitBC : public PenaltyDirichletBC
{
public:
  static InputParameters validParams();

  PenaltyLimitBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  /// Enum for which direct to enforce the limit
  const enum class ApplyPenaltyWhen { GREATERTHAN, LESSTHAN } _apply_penalty_when;
};
