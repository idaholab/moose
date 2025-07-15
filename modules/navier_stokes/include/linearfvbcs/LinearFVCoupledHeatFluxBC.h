//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionBC.h"

/**
 * Class implementing a Neumann boundary condition for linear finite
 * volume variables. This is only applicable for advection-diffusion problems.
 */
class LinearFVCoupledHeatFluxBC : public LinearFVAdvectionDiffusionBC
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVCoupledHeatFluxBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

  virtual bool includesMaterialPropertyMultiplier() const override { return true; }

  Real computeCoupledGradientValue() const;
protected:
  const Moose::Functor<Real> & _vhtc;

  const Moose::Functor<Real> & _solid_conductivity;

  const Moose::Functor<Real> & _fluid_conductivity;

  /// The fluid temperature, we use the functor form to enable situations when
  /// the user wants to supply a solution-independent form for this.
  const Moose::Functor<Real> & _temp_fluid;

  /// The solid/wall temperature, we use the functor form to enable situations when
  /// the user wants to supply a solution-independent form for this.
  const Moose::Functor<Real> & _temp_solid;

  const Moose::Functor<Real> * _rhs_temperature;

  const Moose::Functor<Real> * _rhs_conductivity;

  /// Helper boolean to see if the variable we have is the fluid variable
  bool _var_is_fluid;
};
