/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef CHANGEOVERTIMEPOSTPROCESSOR_H
#define CHANGEOVERTIMEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class ChangeOverTimePostprocessor;

template <>
InputParameters validParams<ChangeOverTimePostprocessor>();

/**
 * Computes the change in a post-processor value, or the magnitude of its
 * relative change, over a time step or over the entire transient.
 */
class ChangeOverTimePostprocessor : public GeneralPostprocessor
{
public:
  ChangeOverTimePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

protected:
  /// option to compute change with respect to initial value instead of previous time value
  const bool _change_with_respect_to_initial;

  /// option to compute the magnitude of relative change instead of change
  const bool _compute_relative_change;

  /// option to take the absolute value of the change
  const bool _take_absolute_value;

  /// current post-processor value
  const PostprocessorValue & _pps_value;

  /// old post-processor value
  const PostprocessorValue & _pps_value_old;

  /// initial post-processor value
  Real _pps_value_initial;
};

#endif /* CHANGEOVERTIMEPOSTPROCESSOR_H */
