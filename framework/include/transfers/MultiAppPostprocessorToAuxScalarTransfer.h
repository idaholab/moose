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
 * Copies the value of a Postprocessor from one app to a scalar AuxVariable in another.
 */
class MultiAppPostprocessorToAuxScalarTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppPostprocessorToAuxScalarTransfer(const InputParameters & parameters);

  /// Execute the transfer
  virtual void execute() override;

protected:
  /// The name of the postprocessor that the transfer originates
  PostprocessorName _from_pp_name;

  /// The name of the field variable to which the postprocessor is being transfered
  VariableName _to_aux_name;
};
