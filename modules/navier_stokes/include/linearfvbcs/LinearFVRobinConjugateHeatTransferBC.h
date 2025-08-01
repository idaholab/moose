//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVConjugateHeatTransferBCBase.h"

class LinearFVRobinConjugateHeatTransferBC : public LinearFVConjugateHeatTransferBCBase
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVRobinConjugateHeatTransferBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void initialSetup() override;

protected:
  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

protected:
  const Moose::Functor<Real> * _incoming_flux;

  /// The fluid temperature, we use the functor form to enable situations when
  /// the user wants to supply a solution-independent form for this.
  const Moose::Functor<Real> * _temp_fluid;

  /// The solid/wall temperature, we use the functor form to enable situations when
  /// the user wants to supply a solution-independent form for this.
  const Moose::Functor<Real> * _temp_solid;

  /// The convective heat transfer coefficient
  const Moose::Functor<Real> & _htc;

  /// Helper boolean to see if the variable we have is the fluid variable
  bool _var_is_fluid;

  /// The temperature which will contribute to the right hand side.
  /// When this is the fluid domain, the solid temperature will go to the
  /// right hand side. When it is the solid domain, the fluid temperature
  /// will contribute to the right hand side.
  const Moose::Functor<Real> * _rhs_temperature;
};
