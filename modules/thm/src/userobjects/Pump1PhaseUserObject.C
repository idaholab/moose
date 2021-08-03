#include "Pump1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndices3Eqn.h"
#include "VolumeJunction1Phase.h"
#include "NumericalFlux3EqnBase.h"
#include "Numerics.h"

registerMooseObject("THMApp", Pump1PhaseUserObject);

InputParameters
Pump1PhaseUserObject::validParams()
{
  InputParameters params = VolumeJunction1PhaseUserObject::validParams();

  params.addRequiredParam<Real>("head", "Pump head, [m]");
  params.addRequiredParam<Real>("gravity_magnitude", "Gravity constant, [m/s^2]");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase pump");

  params.declareControllable("head");

  return params;
}

Pump1PhaseUserObject::Pump1PhaseUserObject(const InputParameters & params)
  : VolumeJunction1PhaseUserObject(params),

    _head(getParam<Real>("head")),
    _g(getParam<Real>("gravity_magnitude"))
{
}

void
Pump1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  VolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  const Point di = _dir[0];
  const RealVectorValue rhouV_vec(_rhouV[0], _rhovV[0], _rhowV[0]);

  // compute momentum and energy source terms
  const RealVectorValue S_momentum = 0.5 * (_rhoV[0] / _volume) * _g * _head * _A_ref * di;
  const Real S_energy = 0.5 * ((rhouV_vec * di) / _volume) * _g * _head * _A_ref;

  // compute momentum and energy source term derivatives w.r.t. scalar variables
  const RealVectorValue dS_momentum_drhoV = (0.5 / _volume) * _g * _head * _A_ref * di;

  const Real dS_energy_drhouV = (0.5 / _volume) * di(0) * _g * _head * _A_ref;
  const Real dS_energy_drhovV = (0.5 / _volume) * di(1) * _g * _head * _A_ref;
  const Real dS_energy_drhowV = (0.5 / _volume) * di(2) * _g * _head * _A_ref;

  _residual[VolumeJunction1Phase::RHOUV_INDEX] -= S_momentum(0);
  _residual[VolumeJunction1Phase::RHOVV_INDEX] -= S_momentum(1);
  _residual[VolumeJunction1Phase::RHOWV_INDEX] -= S_momentum(2);
  _residual[VolumeJunction1Phase::RHOEV_INDEX] -= S_energy;

  // add Jacobian entries for momentum and energy sources
  _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOUV_INDEX](0, 0) -= dS_momentum_drhoV(0);
  _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOVV_INDEX](0, 0) -= dS_momentum_drhoV(1);
  _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOWV_INDEX](0, 0) -= dS_momentum_drhoV(2);
  _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOEV_INDEX](0, 1) -= dS_energy_drhouV;
  _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOEV_INDEX](0, 2) -= dS_energy_drhovV;
  _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOEV_INDEX](0, 3) -= dS_energy_drhowV;
}
