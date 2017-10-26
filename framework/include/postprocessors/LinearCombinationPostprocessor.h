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
#ifndef LINEARCOMBINATIONPOSTPROCESSOR_H
#define LINEARCOMBINATIONPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class LinearCombinationPostprocessor;

template <>
InputParameters validParams<LinearCombinationPostprocessor>();

/**
 * Computes a linear combination between an arbitrary number of post-processors
 *
 * This computes a linear combination of post-processors \f$x_i\f$:
 * \f[
 *   y = \sum\limits_i c_i x_i + b \,,
 * \f]
 * where \f$c_i\f$ is the combination coefficient for \f$x_i\f$ and \f$b\f$
 * is an additional value to add to the sum.
 */
class LinearCombinationPostprocessor : public GeneralPostprocessor
{
public:
  LinearCombinationPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  /// List of names of post-processors to include in linear combination
  const std::vector<PostprocessorName> & _pp_names;
  /// Number of post-processors in linear combination
  const unsigned int _n_pp;
  /// List of linear combination coefficients for each post-processor value
  const std::vector<Real> & _pp_coefs;
  /// Additional value to add to sum
  const Real _b_value;

  /// List of post-processor values
  std::vector<const PostprocessorValue *> _pp_values;
};

#endif /* LINEARCOMBINATIONPOSTPROCESSOR_H */
