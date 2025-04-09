//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionExtrapolatedBC.h"

/**
 * Class describing a convective heat transfer between two domains.
 * The heat flux is described  by:
 * h * (T_solid - T_liquid),
 * where h is the heat transfer coefficient, while T_solid and T_liquid
 * denot the solid and liquid temperatures, respecitvely.
 */
class LinearFVConvectiveHeatTransferBC : public LinearFVAdvectionDiffusionBC
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVConvectiveHeatTransferBC(const InputParameters & parameters);

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

protected:

  /// The fluid temperature variable
  const MooseLinearVariableFV<Real> * _temp_fluid;

  /// The solid/wall temperature variable
  const MooseLinearVariableFV<Real> * _temp_solid;

  /// The convective heat transfer coefficient in the local element
  const Moose::Functor<Real> & _htc;

  /// Helper boolean to see if the variable we have is the fluid variable
  bool _var_is_fluid;

  /// The temperature which will contribute to the right hand side.
  /// When this is the fluid domain, the solid temperature will go to the
  /// right hand side. When it is the solid domain, the fluid temperature
  /// will contribute to the right hand side.
  const MooseLinearVariableFV<Real> * _rhs_temperature;
};
