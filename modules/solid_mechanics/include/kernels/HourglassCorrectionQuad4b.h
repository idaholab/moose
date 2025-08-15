#pragma once

#include "Kernel.h"
#include "libmesh/point.h"

/**
 * Advanced hourglass correction for QUAD4 elements that computes the correction
 * for a single displacement component (x or y). It uses the current geometry (via
 * _current_elem) to compute an affine displacement field, isolates the hourglass modes,
 * and scales the penalty dynamically. This implementation leverages libMesh::Point's
 * overloaded operators to simplify vector arithmetic.
 */
class HourglassCorrectionQuad4b : public Kernel
{
public:
  static InputParameters validParams();

  HourglassCorrectionQuad4b(const InputParameters & parameters);

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

protected:
  /// Base penalty parameter (supplied by the user)
  const Real _penalty;

  /// Shear modulus for stabilization scaling (default 1.0)
  const Real _mu;

  /// Displacement variable (applied component-wise: x or y)
  const MooseVariable::DoFValue & _v;

  /// Hourglass mode vectors for a QUAD4 element (unnormalized)
  /// Mode 1: [ 1, -1,  1, -1 ]
  /// Mode 2: [ 1,  1, -1, -1 ]
  std::vector<Real> _g1;
  std::vector<Real> _g2;
};
