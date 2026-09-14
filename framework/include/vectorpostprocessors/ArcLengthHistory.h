//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

class ArcLengthProblem;

/**
 * Vector postprocessor that records the equilibrium path traced by an ArcLengthProblem, appending
 * the load parameter and a set of postprocessor values as one row per arc-length continuation
 * increment
 *
 * The load parameter the continuation ends at is reached after the last increment is published, so
 * the final row lags it and ArcLengthLoadParameter executed on EXEC_TIMESTEP_END is what reports
 * that value.
 */
class ArcLengthHistory : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  ArcLengthHistory(const InputParameters & parameters);

  /**
   * Checks that every sampled postprocessor executes on ARC_LENGTH_INCREMENT
   */
  virtual void initialSetup() override;

  /**
   * Keeps the rows recorded by the previous increments, because this object holds the complete
   * continuation history
   */
  virtual void initialize() override;

  /**
   * Appends one row: the increment index, the load parameter, and each sampled postprocessor value
   */
  virtual void execute() override;

private:
  /// The problem tracing the equilibrium path, which supplies the load parameter
  const ArcLengthProblem * const _arc_length_problem;

  /// The names of the postprocessors sampled at every continuation increment
  const std::vector<PostprocessorName> & _pp_names;

  /// The continuation increment index of every recorded row
  VectorPostprocessorValue & _increment;

  /// The load parameter of every recorded row
  VectorPostprocessorValue & _lambda;

  /// The values of the sampled postprocessors, ordered as in 'postprocessors'
  std::vector<const PostprocessorValue *> _pp_values;

  /// The recorded history of each sampled postprocessor, ordered as in 'postprocessors'
  std::vector<VectorPostprocessorValue *> _pp_history;
};
