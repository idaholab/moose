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

#include "MFEMCoordinateCoefficients.h"
#include <memory>

/**
 * Class that constructs and owns the scalar cylindrical built-in MFEM coefficients
 * (r, 1/r, 2*pi*r, and measure weight) for cylindrical and axisymmetric MFEM formulations
 */
class MFEMCylindrical : public MFEMCoordinateCoefficients
{
public:
  static InputParameters validParams();

  MFEMCylindrical(const InputParameters & parameters);

  /// Declare all cylindrical built-in scalar coefficients into the coefficient manager
  virtual void declareCoefficients(Moose::MFEM::CoefficientManager & coeffs) override;

protected:
  /// Declare the buit-in cylindrical coefficients into the coefficient manager
  void declareRadialCoefficient(Moose::MFEM::CoefficientManager & coeffs);
  void declareInverseRadialCoefficient(Moose::MFEM::CoefficientManager & coeffs);
  void declareTwoPiRCoefficient(Moose::MFEM::CoefficientManager & coeffs);

  ///MFEM Coefficients for cylindrical coordinate objects owned by this class
  std::unique_ptr<mfem::Coefficient> _r_coeff;
  std::unique_ptr<mfem::Coefficient> _inv_r_coeff;
  std::unique_ptr<mfem::Coefficient> _two_pi_r_coeff;
};

#endif
