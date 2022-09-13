//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppTransfer.h"

/**
 * Copies the value of a Postprocessor either:
 * - from the parent application to a MultiApp (all its subapps)
 * or
 * - from all child apps of a MultiApp to the parent app, performing a reduction operation
 */
class MultiAppPostprocessorTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppPostprocessorTransfer(const InputParameters & parameters);

  virtual void execute() override;

  enum
  {
    AVERAGE,
    SUM,
    MAXIMUM,
    MINIMUM
  };

protected:
  /// Name of the postprocessor to transfer data from
  PostprocessorName _from_pp_name;

  /// Name of the postprocessor to transfer data to
  PostprocessorName _to_pp_name;

  /// Reduction operation to perform when transferring from multiple child apps to the parent app
  MooseEnum _reduction_type;
};
