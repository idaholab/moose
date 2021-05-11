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

namespace Moose
{
namespace FV
{
class Limiter;
}
}
class SinglePhaseFluidProperties;

class PCNSFVKT : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PCNSFVKT(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const SinglePhaseFluidProperties & _fluid;
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

  const ADMaterialProperty<Real> & _T_fluid_elem;
  const ADMaterialProperty<Real> & _T_fluid_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_T_fluid_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_T_fluid_neighbor;
  const ADMaterialProperty<Real> & _pressure_elem;
  const ADMaterialProperty<Real> & _pressure_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_pressure_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_pressure_neighbor;
  const MaterialProperty<Real> & _eps_elem;
  const MaterialProperty<Real> & _eps_neighbor;
  const MooseEnum _eqn;
  const unsigned int _index;
  const MooseArray<ADReal> & _scalar_elem;
  const MooseArray<ADReal> & _scalar_neighbor;

  std::unique_ptr<Moose::FV::Limiter> _limiter;
};
