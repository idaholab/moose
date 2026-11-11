//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Eigenvalues.h"

// MOOSE includes
#include "NonlinearEigenSystem.h"

#include "libmesh/libmesh_config.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>

registerMooseObject("MooseApp", Eigenvalues);

InputParameters
Eigenvalues::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "Returns the eigenvalues from the nonlinear eigen system, optionally with "
      "the angular frequency, frequency, and period of each.");
  params.addParam<bool>("inverse_eigenvalue", false, "True to evaluate the inverse of eigenvalues");
  params.addParam<bool>(
      "natural_frequency",
      false,
      "True to also output the angular frequency, the frequency, and the period of each mode, "
      "taking the real part of each eigenvalue as the square of the angular frequency. The units "
      "are rad/s, Hz, and s in a consistent unit system whose time unit is the second. When "
      "false, no additional vectors are declared.");
  // An iterative eigensolver returns a rigid body eigenvalue as roundoff-sized noise of either
  // sign, near machine epsilon (about 1e-16) times the largest eigenvalue, amplified by the solver
  // tolerance. A relative 1e-10 sits well above that noise and well below the ratio of the lowest
  // to the highest elastic eigenvalue of a typical mesh.
  params.addRangeCheckedParam<Real>(
      "zero_eigenvalue_tolerance",
      1e-10,
      "zero_eigenvalue_tolerance >= 0",
      "Relative tolerance, scaled by the largest absolute real part of the converged eigenvalues, "
      "below which an eigenvalue is treated as a zero (rigid body) mode. Used only when "
      "'natural_frequency' is true.");
  return params;
}

Eigenvalues::Eigenvalues(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _inverse(getParam<bool>("inverse_eigenvalue")),
    _natural_frequency(getParam<bool>("natural_frequency")),
    _zero_eigenvalue_tolerance(getParam<Real>("zero_eigenvalue_tolerance")),
    _eigen_values_real(declareVector("eigen_values_real")),
    _eigen_values_imag(declareVector("eigen_values_imag")),
    _angular_frequency(_natural_frequency ? &declareVector("angular_frequency") : nullptr),
    _frequency(_natural_frequency ? &declareVector("frequency") : nullptr),
    _period(_natural_frequency ? &declareVector("period") : nullptr),
    _nl_eigen(dynamic_cast<const NonlinearEigenSystem *>(&_sys))
{
  if (!_nl_eigen)
    mooseError("Given system is not a NonlinearEigenSystem \n");

  if (_natural_frequency && _inverse)
    paramError("natural_frequency",
               "'natural_frequency' cannot be used together with 'inverse_eigenvalue' because the "
               "inverse of an eigenvalue has no frequency meaning.");
}

void
Eigenvalues::initialize()
{
  _eigen_values_real.clear();
  _eigen_values_imag.clear();
  if (_natural_frequency)
  {
    _angular_frequency->clear();
    _frequency->clear();
    _period->clear();
  }
}

void
Eigenvalues::execute()
{
#ifdef LIBMESH_HAVE_SLEPC
  const std::vector<std::pair<Real, Real>> & eigenvalues = _nl_eigen->getAllConvergedEigenvalues();
  unsigned int n_converged_eigenvalues = eigenvalues.size();
  _eigen_values_real.resize(n_converged_eigenvalues);
  _eigen_values_imag.resize(n_converged_eigenvalues);
  if (_natural_frequency)
  {
    _angular_frequency->resize(n_converged_eigenvalues);
    _frequency->resize(n_converged_eigenvalues);
    _period->resize(n_converged_eigenvalues);
  }
  // Absolute zero tolerance: the relative tolerance scaled by the largest absolute real part of
  // the converged eigenvalues
  Real zero_tolerance = 0.;
  if (_natural_frequency)
  {
    Real max_abs_real = 0.;
    for (const auto & [lambda_r, _] : eigenvalues)
      max_abs_real = std::max(max_abs_real, std::abs(lambda_r));
    zero_tolerance = _zero_eigenvalue_tolerance * max_abs_real;
  }
  unsigned int n_invalid = 0;
  for (unsigned int n = 0; n < n_converged_eigenvalues; n++)
  {
    std::complex<Real> e(eigenvalues[n].first, eigenvalues[n].second);
    std::complex<Real> inv = _inverse ? 1. / e : e;
    _eigen_values_real[n] = inv.real();
    _eigen_values_imag[n] = inv.imag();

    if (_natural_frequency)
    {
      // The frequencies are computed from the eigenvalue as solved, which is the square of the
      // angular frequency for a modal analysis
      const auto & [lambda_r, lambda_i] = eigenvalues[n];
      if (lambda_i != 0. || lambda_r < -zero_tolerance)
      {
        ++n_invalid;
        (*_angular_frequency)[n] = (*_frequency)[n] = (*_period)[n] =
            std::numeric_limits<Real>::quiet_NaN();
      }
      else if (std::abs(lambda_r) <= zero_tolerance)
      {
        // A zero eigenvalue (a rigid body mode) has an infinite period
        (*_angular_frequency)[n] = (*_frequency)[n] = 0.;
        (*_period)[n] = std::numeric_limits<Real>::infinity();
      }
      else
      {
        (*_angular_frequency)[n] = std::sqrt(lambda_r);
        (*_frequency)[n] = (*_angular_frequency)[n] / (2. * libMesh::pi);
        (*_period)[n] = 1. / (*_frequency)[n];
      }
    }
  }

  if (n_invalid)
    mooseWarning(
        n_invalid,
        " eigenvalue(s) have a negative real part or a nonzero imaginary part; the angular "
        "frequency, frequency, and period of those modes are set to NaN.");
#else
  _eigen_values_real.clear();
  _eigen_values_imag.clear();
#endif
}
