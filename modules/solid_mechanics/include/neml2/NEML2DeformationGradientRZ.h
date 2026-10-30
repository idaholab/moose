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
#include "NEML2DeformationGradient.h"

/**
 * Axisymmetric (RZ) version of NEML2DeformationGradient: in addition to the in-plane
 * components, the out-of-plane hoop stretch \f$ F_{\theta\theta} = 1 + u_r / r \f$ is
 * filled in.
 */
class NEML2DeformationGradientRZ : public NEML2DeformationGradient
{
public:
  static InputParameters validParams();

  NEML2DeformationGradientRZ(const InputParameters & parameters);

protected:
  void computeF() override;

  /// Index of the radial coordinate (0 for X, 1 for Y)
  const int64_t _radial_coord;

  /// Radial displacement values at the quadrature points
  const neml2::Tensor * _disp_r;
};

#endif
