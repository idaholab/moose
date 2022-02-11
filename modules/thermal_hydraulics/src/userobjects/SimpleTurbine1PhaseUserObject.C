//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleTurbine1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "VolumeJunction1Phase.h"
#include "THMIndices3Eqn.h"
#include "NumericalFlux3EqnBase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", SimpleTurbine1PhaseUserObject);

InputParameters
SimpleTurbine1PhaseUserObject::validParams()
{
  InputParameters params = VolumeJunction1PhaseUserObject::validParams();
  params.addRequiredParam<bool>("on", "Flag determining if turbine is operating or not");
  params.addRequiredParam<Real>("W_dot", "Power, [W]");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase turbine");

  params.declareControllable("W_dot on");

  return params;
}

SimpleTurbine1PhaseUserObject::SimpleTurbine1PhaseUserObject(const InputParameters & params)
  : VolumeJunction1PhaseUserObject(params),
    _on(getParam<bool>("on")),
    _W_dot(getParam<Real>("W_dot"))
{
}

void
SimpleTurbine1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  VolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  if ((c == 0) && _on)
  {
    const Point di = _dir[0];
    const RealVectorValue rhouV_vec(_rhouV[0], _rhovV[0], _rhowV[0]);

    // energy source
    const Real S_E = _W_dot;
    // momentum source
    const RealVectorValue S_M = _W_dot * _rhoV[0] * di / rhouV_vec.norm();

    _residual[VolumeJunction1Phase::RHOUV_INDEX] += S_M(0);
    _residual[VolumeJunction1Phase::RHOVV_INDEX] += S_M(1);
    _residual[VolumeJunction1Phase::RHOWV_INDEX] += S_M(2);
    _residual[VolumeJunction1Phase::RHOEV_INDEX] += S_E;

    // jacobians
    const RealVectorValue dS_M_drhoV = _W_dot * di / rhouV_vec.norm();

    const RealVectorValue dS_M_drhouV =
        -(_W_dot * _rhouV[0] / std::pow(rhouV_vec * rhouV_vec, 1.5)) * _rhoV[0] * di;
    const RealVectorValue dS_M_drhovV =
        -(_W_dot * _rhovV[0] / std::pow(rhouV_vec * rhouV_vec, 1.5)) * _rhoV[0] * di;
    const RealVectorValue dS_M_drhowV =
        -(_W_dot * _rhowV[0] / std::pow(rhouV_vec * rhouV_vec, 1.5)) * _rhoV[0] * di;

    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOUV_INDEX](0, 0) += dS_M_drhoV(0);
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOVV_INDEX](0, 0) += dS_M_drhoV(1);
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOWV_INDEX](0, 0) += dS_M_drhoV(2);

    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOUV_INDEX](0, 1) += dS_M_drhouV(0);
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOVV_INDEX](0, 1) += dS_M_drhouV(1);
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOWV_INDEX](0, 1) += dS_M_drhouV(2);

    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOUV_INDEX](0, 2) += dS_M_drhovV(0);
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOVV_INDEX](0, 2) += dS_M_drhovV(1);
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOWV_INDEX](0, 2) += dS_M_drhovV(2);

    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOUV_INDEX](0, 3) += dS_M_drhowV(0);
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOVV_INDEX](0, 3) += dS_M_drhowV(1);
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOWV_INDEX](0, 3) += dS_M_drhowV(2);
  }
}
