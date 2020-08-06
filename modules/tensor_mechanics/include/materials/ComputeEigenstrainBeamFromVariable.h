//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainBeamBase.h"

/**
 * ComputeEigenstrainBeamFromVariable computes an eigenstrain from displacement and rotational
 * eigenstrain variables
 */
class ComputeEigenstrainBeamFromVariable : public ComputeEigenstrainBeamBase
{
public:
  static InputParameters validParams();

  ComputeEigenstrainBeamFromVariable(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;

  /// Number of displacement eigenstrain variables
  const unsigned int _ndisp;

  /// Number of rotational eigenstrain variables
  const unsigned int _nrot;

  /// Displacemenet eigenstrain variable values
  const std::vector<const VariableValue *> _disp;

  /// Rotational eigenstrain variable values
  const std::vector<const VariableValue *> _rot;
};
