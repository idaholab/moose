//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSKernel.h"

#define usingAdvectiveFluxKernelMembers                                                  \
  usingKernelMembers;                                                                    \
  using AdvectiveFluxKernel::_eps;                                        \
  using AdvectiveFluxKernel::_grad_eps;                                   \
  using AdvectiveFluxKernel::_density                                     \
  using AdvectiveFluxKernel::_velocity;                                   \
  using AdvectiveFluxKernel::_grad_rho;                                   \
  using AdvectiveFluxKernel::_grad_vel_x;                                 \
  using AdvectiveFluxKernel::_grad_vel_y;                                 \
  using AdvectiveFluxKernel::_grad_vel_z;                                 \
  using AdvectiveFluxKernel::_grad_rho_u;                            \
  using AdvectiveFluxKernel::_grad_rho_v;                            \
  using AdvectiveFluxKernel::_grad_rho_w;                            \
  using AdvectiveFluxKernel::_rz_coord;                                   \
  using AdvectiveFluxKernel::velocityDivergence

class AdvectiveFluxKernel;

declareADValidParams(AdvectiveFluxKernel);

/**
 * Kernel representing the advective component of the system of coupled equations.
 * Derived classes specify the form of the advected field (per unit mass).
 */
class AdvectiveFluxKernel : public CNSKernel
{
public:
  AdvectiveFluxKernel(const InputParameters & parameters);

protected:
  virtual ADReal weakResidual() override;

  /// the advected quantity per unit density
  virtual ADReal advectedField() = 0;

  /// the divergence of velocity
  virtual ADReal velocityDivergence() const;

  /// porosity
  const VariableValue & _eps;

  /// porosity gradient
  const VariableGradient & _grad_eps;

  /// fluid density
  const ADMaterialProperty<Real> & _rho;

  /// velocity
  const ADMaterialProperty<RealVectorValue> & _velocity;

  /// density gradient
  const ADMaterialProperty<RealVectorValue> & _grad_rho;

  /// x-velocity gradient
  const ADMaterialProperty<RealVectorValue> & _grad_vel_x;

  /// y-velocity gradient
  const ADMaterialProperty<RealVectorValue> & _grad_vel_y;

  /// z-velocity gradient
  const ADMaterialProperty<RealVectorValue> & _grad_vel_z;

  /// x-momentum gradient
  const ADMaterialProperty<RealVectorValue> & _grad_rho_u;

  /// y-momentum gradient
  const ADMaterialProperty<RealVectorValue> & _grad_rho_v;

  /// z-momentum gradient
  const ADMaterialProperty<RealVectorValue> & _grad_rho_w;

};
