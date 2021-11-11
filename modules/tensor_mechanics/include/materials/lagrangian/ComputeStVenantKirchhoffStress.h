//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressPK2.h"
#include "GuaranteeConsumer.h"

/// St. Venant-Kirchhoff hyperelasticity
///
/// St. Venant-Kirchhoff hyperelasticity derivative from the
/// strain energy function W = lambda / 2 tr(E)^2 + mu tr(E^2)i
///
/// This is basically "linear elasticity with the Green-Lagrange strain"
/// Therefore, we use the ComputeElasticityTensor system to get the
/// tensor but enforce it to be isotropic with the Guarantee system
/// as this is only a hyperelastic model for an isotropic tensor.
///
class ComputeStVenantKirchhoffStress : public ComputeLagrangianStressPK2, public GuaranteeConsumer
{
public:
  static InputParameters validParams();
  ComputeStVenantKirchhoffStress(const InputParameters & parameters);

protected:
  /// Setup function, used to check on isotropy
  virtual void initialSetup() override;

  /// Actual stress/Jacobian update
  virtual void computeQpPK2Stress() override;

protected:
  const MaterialPropertyName _elasticity_tensor_name;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
};
