//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"
#include <memory>
#include <utility>

namespace Moose
{
namespace FV
{
template <typename>
class Limiter;
}
}
class SinglePhaseFluidProperties;

/**
 * Implements the centered Kurganov-Tadmor discretization of advective fluxes
 */
class PCNSFVKT : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PCNSFVKT(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;
  std::pair<ADReal, ADReal> computeAlphaAndOmega(const ADReal & u_elem_normal,
                                                 const ADReal & u_neighbor_normal,
                                                 const ADReal & c_elem,
                                                 const ADReal & c_neighbor) const;
  static ADReal computeFaceFlux(const ADReal & alpha,
                                const ADReal & omega,
                                const ADReal & sup_vel_elem_normal,
                                const ADReal & sup_vel_neighbor_normal,
                                const ADReal & adv_quant_elem,
                                const ADReal & adv_quant_neighbor);

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;

  ///@{ superficial velocities left == elem, right == neighbor
  const ADMaterialProperty<Real> & _sup_vel_x_elem;
  const ADMaterialProperty<Real> & _sup_vel_x_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_x_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_x_neighbor;
  const ADMaterialProperty<Real> & _sup_vel_y_elem;
  const ADMaterialProperty<Real> & _sup_vel_y_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_y_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_y_neighbor;
  const ADMaterialProperty<Real> & _sup_vel_z_elem;
  const ADMaterialProperty<Real> & _sup_vel_z_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_z_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_z_neighbor;
  ///@}

  ///@{ fluid temperature left == elem, right == neighbor
  const ADMaterialProperty<Real> & _T_fluid_elem;
  const ADMaterialProperty<Real> & _T_fluid_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_T_fluid_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_T_fluid_neighbor;
  ///@}

  ///@{ pressure left == elem, right == neighbor
  const ADMaterialProperty<Real> & _pressure_elem;
  const ADMaterialProperty<Real> & _pressure_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_pressure_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_pressure_neighbor;
  ///@}

  ///@{ porosity left == elem, right == neighbor
  const MaterialProperty<Real> & _eps_elem;
  const MaterialProperty<Real> & _eps_neighbor;
  ///@}

  /// The equation we are solving, e.g. mass, momentum, fluid energy, or passive scalar
  const MooseEnum _eqn;

  /// When solving the momentum equation, the momentum component we are solving for
  const unsigned int _index;

  ///@{ passive scalar values left == elem, right == neighbor
  const ADVariableValue & _scalar_elem;
  const ADVariableValue & _scalar_neighbor;
  const ADVariableGradient * const _grad_scalar_elem;
  const ADVariableGradient * const _grad_scalar_neighbor;
  ///@}

  /// The slope limiter we will apply when interpolating from cell centroids to faces
  std::unique_ptr<Moose::FV::Limiter<ADReal>> _limiter;

  /// Whether to use the Kurganov, Noelle, and Petrova method to compute the omega parameter for
  /// stabilization. If false, then the Kurganov-Tadmor method will be used
  const bool _knp_for_omega;
};
