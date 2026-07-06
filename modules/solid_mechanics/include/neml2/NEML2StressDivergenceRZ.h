//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED

// MOOSE includes
#include "NEML2StressDivergence.h"

/**
 * Axisymmetric (RZ) version of NEML2StressDivergence: the weak form additionally contains the
 * hoop term \f$ \psi \sigma_{\theta\theta} / r \f$ on the radial residual component.
 */
class NEML2StressDivergenceRZ : public NEML2StressDivergence
{
public:
  static InputParameters validParams();

  NEML2StressDivergenceRZ(const InputParameters & parameters);

protected:
  void forward() override;

  /// Index of the radial coordinate (0 for X, 1 for Y)
  const int64_t _radial_coord;

  /// Test functions of the radial displacement variable
  const neml2::Tensor * _test_r;
};

#endif
