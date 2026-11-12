//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NaturalFrequencies.h"

#include <algorithm>
#include <cmath>
#include <limits>

registerMooseObject("SolidMechanicsApp", NaturalFrequencies);

InputParameters
NaturalFrequencies::validParams()
{
  InputParameters params = Eigenvalues::validParams();
  params.addClassDescription(
      "Reports the eigenvalues of a modal analysis together with the angular frequency, "
      "frequency, and period of each mode, taking the real part of each eigenvalue as the square "
      "of the angular frequency.");
  // The inverse of an eigenvalue has no frequency meaning, so it keeps its default of false
  params.suppressParameter<bool>("inverse_eigenvalue");
  // An iterative eigensolver returns a rigid body eigenvalue as roundoff-sized noise of either
  // sign, near machine epsilon (about 1e-16) times the largest eigenvalue, amplified by the solver
  // tolerance. A relative 1e-10 sits well above that noise and well below the ratio of the lowest
  // to the highest elastic eigenvalue of a typical mesh.
  params.addRangeCheckedParam<Real>(
      "zero_eigenvalue_tolerance",
      1e-10,
      "zero_eigenvalue_tolerance >= 0",
      "Relative tolerance, scaled by the largest absolute real part of the converged eigenvalues, "
      "below which an eigenvalue is treated as a zero (rigid body) mode.");
  return params;
}

NaturalFrequencies::NaturalFrequencies(const InputParameters & parameters)
  : Eigenvalues(parameters),
    _zero_eigenvalue_tolerance(getParam<Real>("zero_eigenvalue_tolerance")),
    _angular_frequency(declareVector("angular_frequency")),
    _frequency(declareVector("frequency")),
    _period(declareVector("period"))
{
}

void
NaturalFrequencies::initialize()
{
  Eigenvalues::initialize();
  _angular_frequency.clear();
  _frequency.clear();
  _period.clear();
}

void
NaturalFrequencies::execute()
{
  Eigenvalues::execute();

  const auto n_eigenvalues = _eigen_values_real.size();
  _angular_frequency.resize(n_eigenvalues);
  _frequency.resize(n_eigenvalues);
  _period.resize(n_eigenvalues);

  // Absolute zero tolerance: the relative tolerance scaled by the largest absolute real part of
  // the converged eigenvalues
  Real max_abs_real = 0.;
  for (const auto lambda_r : _eigen_values_real)
    max_abs_real = std::max(max_abs_real, std::abs(lambda_r));
  const Real zero_tolerance = _zero_eigenvalue_tolerance * max_abs_real;

  unsigned int n_invalid = 0;
  for (const auto n : index_range(_eigen_values_real))
  {
    // The eigenvalue as solved is the square of the angular frequency for a modal analysis
    const Real lambda_r = _eigen_values_real[n];
    if (_eigen_values_imag[n] != 0. || lambda_r < -zero_tolerance)
    {
      ++n_invalid;
      _angular_frequency[n] = _frequency[n] = _period[n] = std::numeric_limits<Real>::quiet_NaN();
    }
    else if (std::abs(lambda_r) <= zero_tolerance)
    {
      // A zero eigenvalue (a rigid body mode) has an infinite period
      _angular_frequency[n] = _frequency[n] = 0.;
      _period[n] = std::numeric_limits<Real>::infinity();
    }
    else
    {
      _angular_frequency[n] = std::sqrt(lambda_r);
      _frequency[n] = _angular_frequency[n] / (2. * libMesh::pi);
      _period[n] = 1. / _frequency[n];
    }
  }

  if (n_invalid)
    mooseWarning(
        n_invalid,
        " eigenvalue(s) have a negative real part or a nonzero imaginary part; the angular "
        "frequency, frequency, and period of those modes are set to NaN.");
}
