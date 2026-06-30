//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MooseParsedFunction.h"
#include "MFEMProblem.h"

/**
 * Function object that declares MFEM coordinate-dependent coefficients.
 *
 * Currently only coord_type = RZ is supported. When constructed, this object
 * declares:
 *   <name>_r
 *   <name>_inv_r
 *   <name>_two_pi_r
 */
class MFEMCoordinateTransformations : public MooseParsedFunction
{
public:
  static InputParameters validParams();

  MFEMCoordinateTransformations(const InputParameters & parameters);
  virtual ~MFEMCoordinateTransformations() = default;

  const MooseEnum & coordType() const { return _coord_type; }
  Real invREps() const { return _inv_r_eps; }

protected:
  /// reference to the MFEMProblem instance
  MFEMProblem & _mfem_problem;

  /// Coordinate system type
  const MooseEnum _coord_type;

  /// Regularization used in inv_r = 1 / sqrt(r^2 + eps^2)
  const Real _inv_r_eps;
};

#endif
