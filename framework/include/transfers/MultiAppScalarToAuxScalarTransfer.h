//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MultiAppTransfer.h"

/**
 * Copies the value of a SCALAR variable from one App to another.
 */
class MultiAppScalarToAuxScalarTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppScalarToAuxScalarTransfer(const InputParameters & parameters);

  /// Execute the transfer
  virtual void execute() override;

protected:
  /// The name of the scalar variable from which the values are being transfered
  VariableName _from_variable_name;

  /// The name of the auxiliary scalar variable to which the scalar values are being transfered
  VariableName _to_aux_name;
};
