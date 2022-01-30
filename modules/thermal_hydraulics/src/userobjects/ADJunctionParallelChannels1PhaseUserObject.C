#include "ADJunctionParallelChannels1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndices3Eqn.h"
#include "VolumeJunction1Phase.h"
#include "NumericalFlux3EqnBase.h"
#include "Numerics.h"
#include "LoggingInterface.h"

registerMooseObject("ThermalHydraulicsApp", ADJunctionParallelChannels1PhaseUserObject);

InputParameters
ADJunctionParallelChannels1PhaseUserObject::validParams()
{
  InputParameters params = ADVolumeJunction1PhaseUserObject::validParams();
  params.addRequiredParam<std::string>("component_name", "Name of the associated component");
  params.addRequiredParam<RealVectorValue>("dir_c0", "Direction of the first connection");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase junction "
                             "that connects flow channels that are parallel");

  return params;
}

ADJunctionParallelChannels1PhaseUserObject::ADJunctionParallelChannels1PhaseUserObject(
    const InputParameters & params)
  : ADVolumeJunction1PhaseUserObject(params),

    _p(getADMaterialProperty<Real>("p")),

    _dir_c0(getParam<RealVectorValue>("dir_c0")),

    _stored_pA(_n_connections),
    _areas(_n_connections),
    _directions(_n_connections),
    _is_inlet(_n_connections),
    _connection_indices(_n_connections),

    _component_name(getParam<std::string>("component_name"))
{
}

void
ADJunctionParallelChannels1PhaseUserObject::initialize()
{
  ADVolumeJunctionBaseUserObject::initialize();

  _connection_indices.clear();
}

void
ADJunctionParallelChannels1PhaseUserObject::storeConnectionData()
{
  ADVolumeJunctionBaseUserObject::storeConnectionData();

  const unsigned int c = getBoundaryIDIndex();
  _connection_indices.push_back(c);
}

void
ADJunctionParallelChannels1PhaseUserObject::execute()
{
  storeConnectionData();

  const unsigned int c = getBoundaryIDIndex();

  computeFluxesAndResiduals(c);
}

void
ADJunctionParallelChannels1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  ADVolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  const Real din = _normal[c];
  const Point di = _dir[0];
  const Point ni = di * din;

  const ADReal vJ = THM::v_from_rhoA_A(_rhoV[0], _volume);
  const ADRealVectorValue rhouV_vec(_rhouV[0], _rhovV[0], _rhowV[0]);
  const ADReal rhouV2 = rhouV_vec * rhouV_vec;
  const ADReal eJ = _rhoEV[0] / _rhoV[0] - 0.5 * rhouV2 / (_rhoV[0] * _rhoV[0]);
  const ADReal pJ = _fp.p_from_v_e(vJ, eJ);

  _residual[VolumeJunction1Phase::RHOUV_INDEX] -= pJ * ni(0) * _A[0];
  _residual[VolumeJunction1Phase::RHOVV_INDEX] -= pJ * ni(1) * _A[0];
  _residual[VolumeJunction1Phase::RHOWV_INDEX] -= pJ * ni(2) * _A[0];

  if (c == 0)
  {
    if (MooseUtils::absoluteFuzzyEqual(_rhouA[0], 0))
      _d_flow = _dir[0];
    else
      _d_flow = _dir[0] * _rhouA[0] / std::abs(_rhouA[0]);
  }

  if (!THM::areParallelVectors(_dir_c0, _dir[0]))
    mooseError(_component_name,
               ": Connected flow channels are not parallel, use VolumeJunction1Phase "
               "component instead.");

  _areas[c] = _A[0];
  _directions[c] = _dir[0];
  _is_inlet[c] = THM::isOutlet(_rhouA[0], _normal[c]);
  _stored_pA[c] = _p[0] * _A[0];
}

void
ADJunctionParallelChannels1PhaseUserObject::threadJoin(const UserObject & uo)
{
  ADVolumeJunctionBaseUserObject::threadJoin(uo);

  const ADJunctionParallelChannels1PhaseUserObject & jpc_uo =
      dynamic_cast<const ADJunctionParallelChannels1PhaseUserObject &>(uo);

  // Store the data computed/retrieved in the other threads
  for (unsigned int i = 0; i < jpc_uo._connection_indices.size(); i++)
  {
    const unsigned int c = jpc_uo._connection_indices[i];

    _areas[c] = jpc_uo._areas[c];
    _directions[c] = jpc_uo._directions[c];
    _is_inlet[c] = jpc_uo._is_inlet[c];
    _stored_pA[c] = jpc_uo._stored_pA[c];
    if (c == 0)
      _d_flow = jpc_uo._d_flow;
  }
}

void
ADJunctionParallelChannels1PhaseUserObject::finalize()
{
  ADVolumeJunctionBaseUserObject::finalize();

  ADReal Ain_total = 0;
  ADReal pAin_total = 0;
  ADReal pAout_total = 0;
  ADReal Aout_total = 0;
  _c_in.clear();
  _c_out.clear();
  for (unsigned int c = 0; c < _n_connections; c++)
  {
    if (_is_inlet[c] == true)
    {
      Ain_total += _areas[c];
      pAin_total += _stored_pA[c];
      _c_in.push_back(c);
    }
    else
    {
      Aout_total += _areas[c];
      pAout_total += _stored_pA[c];
      _c_out.push_back(c);
    }
  }

  ADReal p_wall = 0;
  ADReal A_wall = 0;
  ADRealVectorValue d_wall;

  if (Aout_total > Ain_total)
  {
    if (Aout_total > 1e-15)
    {
      p_wall = pAout_total / Aout_total;
      _c_wall = _c_out;
    }
    else
    {
      p_wall = 0;
    }
    A_wall = Aout_total - Ain_total;
    d_wall = _d_flow;
  }
  else
  {
    if (Ain_total > 1e-15)
    {
      p_wall = pAin_total / Ain_total;
      _c_wall = _c_in;
    }
    else
    {
      p_wall = 0;
    }
    A_wall = Ain_total - Aout_total;
    d_wall = -_d_flow;
  }

  ADVolumeJunctionBaseUserObject::_residual[VolumeJunction1Phase::RHOUV_INDEX] -=
      d_wall(0) * p_wall * A_wall;
  ADVolumeJunctionBaseUserObject::_residual[VolumeJunction1Phase::RHOVV_INDEX] -=
      d_wall(1) * p_wall * A_wall;
  ADVolumeJunctionBaseUserObject::_residual[VolumeJunction1Phase::RHOWV_INDEX] -=
      d_wall(2) * p_wall * A_wall;
}
