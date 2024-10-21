//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADJunctionParallelChannels1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndicesVACE.h"
#include "VolumeJunction1Phase.h"
#include "NumericalFlux3EqnBase.h"
#include "Numerics.h"
#include "LoggingInterface.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "libmesh/parallel_algebra.h"

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
    _is_inlet(_n_connections),

    _component_name(getParam<std::string>("component_name"))
{
}

void
ADJunctionParallelChannels1PhaseUserObject::initialize()
{
  ADVolumeJunction1PhaseUserObject::initialize();

  _c_in.clear();
  _c_out.clear();
}

void
ADJunctionParallelChannels1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  ADVolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  const Real din = _normal[c];
  const Point di = _dir[0];
  const Point ni = di * din;

  const auto & rhoV = _cached_junction_var_values[VolumeJunction1Phase::RHOV_INDEX];
  const auto & rhouV = _cached_junction_var_values[VolumeJunction1Phase::RHOUV_INDEX];
  const auto & rhovV = _cached_junction_var_values[VolumeJunction1Phase::RHOVV_INDEX];
  const auto & rhowV = _cached_junction_var_values[VolumeJunction1Phase::RHOWV_INDEX];
  const auto & rhoEV = _cached_junction_var_values[VolumeJunction1Phase::RHOEV_INDEX];

  const ADReal vJ = THM::v_from_rhoA_A(rhoV, _volume);
  const ADRealVectorValue rhouV_vec(rhouV, rhovV, rhowV);
  const ADReal rhouV2 = rhouV_vec * rhouV_vec;
  const ADReal eJ = rhoEV / rhoV - 0.5 * rhouV2 / (rhoV * rhoV);
  const ADReal pJ = _fp.p_from_v_e(vJ, eJ);

  _residual[VolumeJunction1Phase::RHOUV_INDEX] -= pJ * ni(0) * _A[0];
  _residual[VolumeJunction1Phase::RHOVV_INDEX] -= pJ * ni(1) * _A[0];
  _residual[VolumeJunction1Phase::RHOWV_INDEX] -= pJ * ni(2) * _A[0];

  if (c == 0)
  {
    if (MooseUtils::absoluteFuzzyEqual(_rhouA[0], 0))
      _d_flow = _dir[0];
    else
      // FIXME: _d_flow should be again ADRealVectorValue when we have parallel comm on
      // ADRealVectorValue
      _d_flow =
          _dir[0] * MetaPhysicL::raw_value(_rhouA[0]) / std::abs(MetaPhysicL::raw_value(_rhouA[0]));
  }

  if (!THM::areParallelVectors(_dir_c0, _dir[0]))
    mooseError(_component_name,
               ": Connected flow channels are not parallel, use VolumeJunction1Phase "
               "component instead.");

  _areas[c] = _A[0];
  _is_inlet[c] = THM::isOutlet(_rhouA[0], _normal[c]);
  _stored_pA[c] = _p[0] * _A[0];
}

void
ADJunctionParallelChannels1PhaseUserObject::threadJoin(const UserObject & uo)
{
  ADVolumeJunction1PhaseUserObject::threadJoin(uo);

  const auto & jpc_uo = static_cast<const ADJunctionParallelChannels1PhaseUserObject &>(uo);

  // Store the data computed/retrieved in the other threads
  for (unsigned int i = 0; i < jpc_uo._connection_indices.size(); i++)
  {
    const unsigned int c = jpc_uo._connection_indices[i];

    _areas[c] = jpc_uo._areas[c];
    _is_inlet[c] = jpc_uo._is_inlet[c];
    _stored_pA[c] = jpc_uo._stored_pA[c];
    if (c == 0)
      _d_flow = jpc_uo._d_flow;
  }
}

void
ADJunctionParallelChannels1PhaseUserObject::finalize()
{
  for (unsigned int i = 0; i < _n_connections; i++)
  {
    processor_id_type owner_proc = _processor_ids[i];
    comm().broadcast(_areas[i], owner_proc, true);
    comm().broadcast(_stored_pA[i], owner_proc, true);
    // because std::vector<bool> is very special
    bool b = _is_inlet[i];
    comm().broadcast(b, owner_proc, true);
    _is_inlet[i] = b;
  }
  comm().broadcast(_d_flow, _processor_ids[0], true);

  ADReal Ain_total = 0;
  ADReal pAin_total = 0;
  ADReal pAout_total = 0;
  ADReal Aout_total = 0;
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
  RealVectorValue d_wall;

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

  for (unsigned int i = 0; i < _n_scalar_eq; i++)
    comm().sum(_residual[i]);

  _residual[VolumeJunction1Phase::RHOUV_INDEX] -= d_wall(0) * p_wall * A_wall;
  _residual[VolumeJunction1Phase::RHOVV_INDEX] -= d_wall(1) * p_wall * A_wall;
  _residual[VolumeJunction1Phase::RHOWV_INDEX] -= d_wall(2) * p_wall * A_wall;
}
