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

#ifndef CUMULATIVEVALUEPOSTPROCESSOR_H
#define CUMULATIVEVALUEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class CumulativeValuePostprocessor;

template <>
InputParameters validParams<CumulativeValuePostprocessor>();

/**
 * Creates a cumulative sum of a post-processor value over a transient.
 *
 * This is useful, for example, for counting the total number of linear or
 * nonlinear iterations during a transient.
 */
class CumulativeValuePostprocessor : public GeneralPostprocessor
{
public:
  CumulativeValuePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

protected:
  /// cumulative sum of the post-processor value
  Real _sum;

  /// cumulative sum of the post-processor value from the old time step */
  const PostprocessorValue & _sum_old;

  /// current post-processor value to be added to the cumulative sum
  const PostprocessorValue & _pps_value;
};

#endif /* CUMULATIVEVALUEPOSTPROCESSOR_H */
