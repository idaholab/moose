//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVCHTBCBase.h"

/**
 * Conjugate heat transfer BC for Dirichlet boundary condition-based
 * coupling. Differs from regular BCs due to the need of error checking
 * and the ability to compute the diffusive flux on a boundary.
 */
class LinearFVDirichletCHTBC : public LinearFVCHTBCBase
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVDirichletCHTBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryConductionFlux() const override;

protected:
  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

  /**
   * Thermal conductivity, mainly used for the computation of the heat flux.
   */
  const Moose::Functor<Real> & _thermal_conductivity;

  /**
   * Functor describing the temperature which is being prescibed on the
   * boundary.
   */
  const Moose::Functor<Real> & _prescribed_temperature;
};
