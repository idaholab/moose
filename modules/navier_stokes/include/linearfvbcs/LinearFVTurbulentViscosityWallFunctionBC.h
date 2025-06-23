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
#include "NS.h"

/**
 * Class implementing a Dirichlet boundary condition for linear finite
 * volume variables. This is only applicable for advection-diffusion problems.
 */
class LinearFVTurbulentViscosityWallFunctionBC : public LinearFVAdvectionDiffusionBC
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVTurbulentViscosityWallFunctionBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

  virtual bool useBoundaryGradientExtrapolation() const override { return true; }

  Real computeTurbulentViscosity() const;

protected:
  /// the dimension of the domain
  const unsigned int _dim;

  /// x-velocity
  const Moose::Functor<Real> & _u_var;
  /// y-velocity
  const Moose::Functor<Real> * _v_var;
  /// z-velocity
  const Moose::Functor<Real> * _w_var;

  /// Density
  const Moose::Functor<Real> & _rho;
  /// Dynamic viscosity
  const Moose::Functor<Real> & _mu;

  /// Turbulent kinetic energy
  const Moose::Functor<Real> & _k;

  /// C_mu turbulent coefficient
  const Real _C_mu;

  /// Method used for wall treatment
  NS::WallTreatmentEnum _wall_treatment;

  // Mu_t evaluated at y+=30 for blending purposes
  const Real _mut_30 =
      (NS::von_karman_constant * 30.0 / std::log(NS::E_turb_constant * 30.0) - 1.0);
};
