//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

/**
 *  VectorOfPostprocessors is a type of VectorPostprocessor that outputs the
 *  values of an arbitrary user-specified set of postprocessors as a vector in the order specified
 * by the user.
 */

class VectorOfPostprocessors : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  VectorOfPostprocessors(const InputParameters & parameters);

  /**
   * Initialize, clears the postprocessor vector
   */
  virtual void initialize() override;

  /**
   * Populates the postprocessor vector of values with the supplied postprocessors
   */
  virtual void execute() override;

protected:
  /// The VectorPostprocessorValue object where the results are stored
  VectorPostprocessorValue & _pp_vec;

  /// The vector of PostprocessorValue objects that are used to get the values of the postprocessors
  std::vector<const PostprocessorValue *> _postprocessor_values;
};
