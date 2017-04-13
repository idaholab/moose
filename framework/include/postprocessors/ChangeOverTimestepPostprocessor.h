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

#ifndef CHANGEOVERTIMESTEPPOSTPROCESSOR_H
#define CHANGEOVERTIMESTEPPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class ChangeOverTimestepPostprocessor;

template <>
InputParameters validParams<ChangeOverTimestepPostprocessor>();

/**
 * Computes the change in a post-processor value, or the magnitude of its
 * relative change, over a time step.
 */
class ChangeOverTimestepPostprocessor : public GeneralPostprocessor
{
public:
  ChangeOverTimestepPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

protected:
  /// option to compute the magnitude of relative change instead of change
  const bool _compute_relative_change;

  /// current post-processor value
  const PostprocessorValue & _pps_value;

  /// old post-processor value
  const PostprocessorValue & _pps_value_old;
};

#endif /* CHANGEOVERTIMESTEPPOSTPROCESSOR_H */
