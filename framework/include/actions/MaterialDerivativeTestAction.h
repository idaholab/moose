//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "DerivativeMaterialPropertyNameInterface.h"

/**
 * Sets up variables and Kernels to test the derivatives of material properties via
 * the Jacobian checker
 */
class MaterialDerivativeTestAction : public Action, public DerivativeMaterialPropertyNameInterface
{
public:
  static InputParameters validParams();

  MaterialDerivativeTestAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  std::vector<VariableName> _args;

  MaterialPropertyName _prop_name;

  enum class PropTypeEnum
  {
    REAL,
    RANKTWOTENSOR,
    RANKFOURTENSOR
  } _prop_type;

  const unsigned int _derivative_order;

  const bool _second;

  /// every derivative given by a list of variables to derive w.r.t
  std::map<MaterialPropertyName, std::vector<SymbolName>> _derivatives;
};
