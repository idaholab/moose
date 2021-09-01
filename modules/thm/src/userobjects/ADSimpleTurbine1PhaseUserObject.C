#include "ADSimpleTurbine1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "VolumeJunction1Phase.h"
#include "THMIndices3Eqn.h"
#include "ADNumericalFlux3EqnBase.h"
#include "Numerics.h"

registerMooseObject("THMApp", ADSimpleTurbine1PhaseUserObject);

InputParameters
ADSimpleTurbine1PhaseUserObject::validParams()
{
  InputParameters params = ADVolumeJunction1PhaseUserObject::validParams();
  params.addRequiredParam<bool>("on", "Flag determining if turbine is operating or not");
  params.addRequiredParam<Real>("W_dot", "Power, [W]");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase turbine");

  params.declareControllable("W_dot on");

  return params;
}

ADSimpleTurbine1PhaseUserObject::ADSimpleTurbine1PhaseUserObject(const InputParameters & params)
  : ADVolumeJunction1PhaseUserObject(params),
    _on(getParam<bool>("on")),
    _W_dot(getParam<Real>("W_dot"))
{
}

void
ADSimpleTurbine1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  ADVolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  if ((c == 0) && _on)
  {
    const Point di = _dir[0];
    const ADRealVectorValue rhouV_vec(_rhouV[0], _rhovV[0], _rhowV[0]);

    // energy source
    const ADReal S_E = _W_dot;
    // momentum source
    const ADRealVectorValue S_M = _W_dot * _rhoV[0] * di / rhouV_vec.norm();

    _residual[VolumeJunction1Phase::RHOUV_INDEX] += S_M(0);
    _residual[VolumeJunction1Phase::RHOVV_INDEX] += S_M(1);
    _residual[VolumeJunction1Phase::RHOWV_INDEX] += S_M(2);
    _residual[VolumeJunction1Phase::RHOEV_INDEX] += S_E;
  }
}
