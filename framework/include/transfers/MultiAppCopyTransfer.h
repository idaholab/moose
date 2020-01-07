//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppFieldTransfer.h"

// Forward declarations
class MultiAppCopyTransfer;

template <>
InputParameters validParams<MultiAppCopyTransfer>();

/**
 * Copy the value to the target domain from the nearest node in the source domain.
 */
class MultiAppCopyTransfer : public MultiAppFieldTransfer
{
public:
  static InputParameters validParams();

  MultiAppCopyTransfer(const InputParameters & parameters);

  /**
   * Performs the transfer of a variable (Nonlinear or Auxiliary) to/from the Multiapp.
   */
  virtual void execute() override;

protected:
  virtual std::vector<VariableName> getFromVarNames() const override { return _from_var_names; }
  virtual std::vector<AuxVariableName> getToVarNames() const override { return _to_var_names; }

  /// Name of variables transfering from
  const std::vector<VariableName> _from_var_names;
  /// Name of variables transfering to
  const std::vector<AuxVariableName> _to_var_names;

  /// This values are used if a derived class only supports one variable
  VariableName _from_var_name;
  AuxVariableName _to_var_name;
};
