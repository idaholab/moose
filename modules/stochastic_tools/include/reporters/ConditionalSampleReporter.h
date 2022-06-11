//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActiveLearningReporterBase.h"
#include "FunctionParserUtils.h"

// Forward declaration
template <typename T>
class ConditionalSampleReporterTempl;

// Typedef for object registration
typedef ConditionalSampleReporterTempl<Real> ConditionalSampleReporter;

/**
 * This object is mainly meant for demonstration for eventual active learning
 * algorithms, but could prove useful. Basically, it enables a inputted function
 * to determine if a multiapp solve is "possible". For instance, maybe a certain
 * sampled value needs to be possitive, this class can filter those samples out
 * and replace quantities of interest with a default value.
 */
template <typename T>
class ConditionalSampleReporterTempl : public ActiveLearningReporterTempl<T>,
                                       public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();
  ConditionalSampleReporterTempl(const InputParameters & parameters);

protected:
  /**
   * This evaluates the inputted function to determine whether a multiapp solve is
   * necessary/allowed, otherwise it replaces the "transferred" quantity with a
   * default value.
   */
  virtual bool needSample(const std::vector<Real> & row,
                          dof_id_type local_ind,
                          dof_id_type global_ind,
                          T & val) override;

private:
  /// User-inputted function string
  const std::string & _function;
  /// User-inputted default value
  const T & _default_value;
  /// Pairs between the variables in the function string and their associated
  /// sampler column index
  const std::vector<std::pair<std::string, unsigned int>> _sampler_vars;
  /// Whether or not the function string includes a time variable
  const bool _use_time;
  /// Parsed function pointer
  SymFunctionPtr _func_F;
};
