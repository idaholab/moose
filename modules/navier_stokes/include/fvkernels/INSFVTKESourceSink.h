//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"
#include "INSFVVelocityVariable.h"
#include "NS.h"

/**
 * Computes source the sink terms for the turbulent kinetic energy.
 */
class INSFVTKESourceSink : public FVElementalKernel
{
public:
  static InputParameters validParams();

  virtual void initialSetup() override;

  INSFVTKESourceSink(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

protected:
  /// The dimension of the domain
  const unsigned int _dim;

  /// x-velocity
  const Moose::Functor<ADReal> & _u_var;
  /// y-velocity
  const Moose::Functor<ADReal> * _v_var;
  /// z-velocity
  const Moose::Functor<ADReal> * _w_var;

  /// epsilon - dissipation rate of TKE
  const Moose::Functor<ADReal> & _epsilon;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// Turbulent dynamic viscosity
  const Moose::Functor<ADReal> & _mu_t;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// Linearized model?
  const bool _linearized_model;

  /// Method used for wall treatment
  NS::WallTreatmentEnum _wall_treatment;

  /// C_mu constant
  const Real _C_mu;

  // Production Limiter Constant
  const Real _C_pl;

  /// For Newton solves we want to add extra zero-valued terms regardless of y-plus to avoid sparsity pattern changes as y-plus changes near the walls
  const bool _newton_solve;

  ///@{
  /// Maps for wall treatement
  std::map<const Elem *, bool> _wall_bounded;
  std::map<const Elem *, std::vector<Real>> _dist;
  std::map<const Elem *, std::vector<const FaceInfo *>> _face_infos;
  ///@}
};
