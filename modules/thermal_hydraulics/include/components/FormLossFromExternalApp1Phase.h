//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FormLoss1PhaseBase.h"

/**
 * A component for prescribing a form loss computed by an external application
 */
class FormLossFromExternalApp1Phase : public FormLoss1PhaseBase
{
public:
  FormLossFromExternalApp1Phase(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// Name of the variable that stores the distributed form loss coefficient
  VariableName _K_prime_var_name;

public:
  static InputParameters validParams();
};
