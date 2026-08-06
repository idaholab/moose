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
#include "NEML2SmallStrain.h"

/**
 * Large-deformation companion of NEML2SmallStrain: computes the deformation gradient
 * \f$ F = I + \nabla_0 u \f$ (reference-configuration gradient, no symmetrization) as a
 * full second-order tensor for consumption by total-Lagrangian NEML2 models.
 *
 * With stabilize_strain = true the F-bar volumetric correction is applied:
 * \f$ F \leftarrow F \, (\det \bar{F} / \det F)^{1/3} \f$ with \f$ \bar{F} \f$ the
 * volume-weighted element average, matching ComputeLagrangianStrainBase.
 */
class NEML2DeformationGradient : public NEML2SmallStrain
{
public:
  static InputParameters validParams();

  NEML2DeformationGradient(const InputParameters & parameters);

protected:
  void forward() override final;

  /// Fill _output with the unstabilized deformation gradient
  virtual void computeF();

  /// F-bar: rescale each quadrature point's F to the element-average volume change
  void applyFBar();

  /// Whether to apply the F-bar volumetric correction
  const bool _stabilize_strain;
};

#endif
