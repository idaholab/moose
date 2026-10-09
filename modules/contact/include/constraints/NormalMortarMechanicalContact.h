//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarLagrangeConstraint.h"

class WeightedGapUserObject;

class NormalMortarMechanicalContact : public ADMortarLagrangeConstraint
{
public:
  static InputParameters validParams();

  NormalMortarMechanicalContact(const InputParameters & parameters);

  void initialSetup() override;

protected:
  void precomputeQpQuantities() override;
  ADReal computeQpResidual(Moose::MortarType type) final;

  /// The interpolated normal traction component at the current quadrature point
  ADReal _qp_traction_component;

  /// The displacement component that this object applies to
  const MooseEnum _component;

  /// The weighted gap user object which supplies the contact force
  const WeightedGapUserObject & _weighted_gap_uo;
};
