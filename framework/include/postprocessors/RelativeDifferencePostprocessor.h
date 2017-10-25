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
#ifndef RELATIVEDIFFERENCEPOSTPROCESSOR_H
#define RELATIVEDIFFERENCEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class RelativeDifferencePostprocessor;

template <>
InputParameters validParams<RelativeDifferencePostprocessor>();

/**
 * Computes the absolute value of the relative difference between 2
 * post-processor values.
 *
 * This post-processor computes the absolute value of the relative difference
 * between 2 post-processor values:
 * \f[
 *   y = \left| \frac{x_1 - x_2}{x_2} \right| \,,
 * \f]
 * where \f$x_1\f$ and \f$x_2\f$ are the 2 post-processor values. Note that
 * \f$x_2\f$ is used as the base for the relative difference. If
 * \f$x_2 \approx 0\f$, then the absolute difference is used instead to prevent
 * division by zero:
 * \f[
 *   y = \left| x_1 - x_2 \right| \,.
 * \f]
 */
class RelativeDifferencePostprocessor : public GeneralPostprocessor
{
public:
  RelativeDifferencePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  /// first post-processor value
  const PostprocessorValue & _value1;
  /// second post-processor value, used as base in relative difference
  const PostprocessorValue & _value2;
};

#endif /* RELATIVEDIFFERENCEPOSTPROCESSOR_H */
