//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

/**
 * A class used to generate samples for a direct perturbation analysis.
 */
class DirectPerturbationSampler : public Sampler
{
public:
  static InputParameters validParams();

  DirectPerturbationSampler(const InputParameters & parameters);

  /// Return the requested perturbation method
  const MooseEnum & perturbationMethod() const { return _perturbation_method; }

  /// Return the absolute perturbation interval for a given index
  Real getAbsoluteInterval(const Real param_index) const;

  /// Return the relative perturbation interval for a given index
  Real getRelativeInterval(const Real param_index) const;

  /// Return the nominal value of the parameter
  Real getNominalValue(const Real param_index) const;

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

private:
  /// The nominal values of the parameters
  const std::vector<Real> _nominal_values;

  /// The relative intervals that should be used for the perturbation of each parameter
  const std::vector<Real> _relative_intervals;

  /// The method which is used for the perturbation (one-sided/two-sided)
  const MooseEnum _perturbation_method;

  /// The data matrix created using the parameters
  std::vector<std::vector<Real>> _parameter_vectors;

  /// The intervals for the perturbations
  std::vector<Real> _absolute_intervals;
};
