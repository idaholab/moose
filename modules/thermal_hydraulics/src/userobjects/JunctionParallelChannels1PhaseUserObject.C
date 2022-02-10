//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JunctionParallelChannels1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndices3Eqn.h"
#include "VolumeJunction1Phase.h"
#include "NumericalFlux3EqnBase.h"
#include "Numerics.h"
#include "LoggingInterface.h"

registerMooseObject("ThermalHydraulicsApp", JunctionParallelChannels1PhaseUserObject);

InputParameters
JunctionParallelChannels1PhaseUserObject::validParams()
{
  InputParameters params = VolumeJunction1PhaseUserObject::validParams();
  params.addRequiredParam<std::string>("component_name", "Name of the associated component");
  params.addRequiredParam<RealVectorValue>("dir_c0", "Direction of the first connection");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase junction "
                             "that connects flow channels that are parallel");

  return params;
}

JunctionParallelChannels1PhaseUserObject::JunctionParallelChannels1PhaseUserObject(
    const InputParameters & params)
  : DerivativeMaterialInterfaceTHM<VolumeJunction1PhaseUserObject>(params),

    _p(getMaterialProperty<Real>("p")),
    _dp_drhoA(getMaterialPropertyDerivativeTHM<Real>("p", "rhoA")),
    _dp_drhouA(getMaterialPropertyDerivativeTHM<Real>("p", "rhouA")),
    _dp_drhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "rhoEA")),

    _dir_c0(getParam<RealVectorValue>("dir_c0")),

    _stored_pA(_n_connections),
    _stored_dp_drhoA(_n_connections),
    _stored_dp_drhouA(_n_connections),
    _stored_dp_drhoEA(_n_connections),
    _areas(_n_connections),
    _directions(_n_connections),
    _is_inlet(_n_connections),
    _connection_indices(_n_connections),

    _component_name(getParam<std::string>("component_name"))
{
}

void
JunctionParallelChannels1PhaseUserObject::initialize()
{
  VolumeJunctionBaseUserObject::initialize();

  _connection_indices.clear();
}

void
JunctionParallelChannels1PhaseUserObject::storeConnectionData()
{
  VolumeJunctionBaseUserObject::storeConnectionData();

  const unsigned int c = getBoundaryIDIndex();
  _connection_indices.push_back(c);
}

void
JunctionParallelChannels1PhaseUserObject::execute()
{
  storeConnectionData();

  const unsigned int c = getBoundaryIDIndex();

  computeFluxesAndResiduals(c);
}

void
JunctionParallelChannels1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  VolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  const Real din = _normal[c];
  const Point di = _dir[0];
  const Point ni = di * din;

  Real vJ, dvJ_drhoV;
  THM::v_from_rhoA_A(_rhoV[0], _volume, vJ, dvJ_drhoV);

  const RealVectorValue rhouV_vec(_rhouV[0], _rhovV[0], _rhowV[0]);
  const Real rhouV2 = rhouV_vec * rhouV_vec;
  const Real eJ = _rhoEV[0] / _rhoV[0] - 0.5 * rhouV2 / (_rhoV[0] * _rhoV[0]);
  const Real deJ_drhoV = -_rhoEV[0] / (_rhoV[0] * _rhoV[0]) + rhouV2 / std::pow(_rhoV[0], 3);
  const Real deJ_drhouV = -_rhouV[0] / (_rhoV[0] * _rhoV[0]);
  const Real deJ_drhovV = -_rhovV[0] / (_rhoV[0] * _rhoV[0]);
  const Real deJ_drhowV = -_rhowV[0] / (_rhoV[0] * _rhoV[0]);
  const Real deJ_drhoEV = 1.0 / _rhoV[0];

  Real pJ, dpJ_dvJ, dpJ_deJ;
  _fp.p_from_v_e(vJ, eJ, pJ, dpJ_dvJ, dpJ_deJ);
  std::vector<Real> dpJ_dUJ(_n_scalar_eq, 0);
  dpJ_dUJ[VolumeJunction1Phase::RHOV_INDEX] = dpJ_dvJ * dvJ_drhoV + dpJ_deJ * deJ_drhoV;
  dpJ_dUJ[VolumeJunction1Phase::RHOUV_INDEX] = dpJ_deJ * deJ_drhouV;
  dpJ_dUJ[VolumeJunction1Phase::RHOVV_INDEX] = dpJ_deJ * deJ_drhovV;
  dpJ_dUJ[VolumeJunction1Phase::RHOWV_INDEX] = dpJ_deJ * deJ_drhowV;
  dpJ_dUJ[VolumeJunction1Phase::RHOEV_INDEX] = dpJ_deJ * deJ_drhoEV;

  _residual[VolumeJunction1Phase::RHOUV_INDEX] -= pJ * ni(0) * _A[0];
  _residual[VolumeJunction1Phase::RHOVV_INDEX] -= pJ * ni(1) * _A[0];
  _residual[VolumeJunction1Phase::RHOWV_INDEX] -= pJ * ni(2) * _A[0];

  for (unsigned int i = 0; i < _n_scalar_eq; i++)
  {
    // Cache scalar residual Jacobian entries w.r.t. scalar variables
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOUV_INDEX](0, i) -=
        dpJ_dUJ[i] * ni(0) * _A[0];
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOVV_INDEX](0, i) -=
        dpJ_dUJ[i] * ni(1) * _A[0];
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOWV_INDEX](0, i) -=
        dpJ_dUJ[i] * ni(2) * _A[0];
  }

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
  _stored_dp_drhoA[c] = _dp_drhoA[0];
  _stored_dp_drhouA[c] = _dp_drhouA[0];
  _stored_dp_drhoEA[c] = _dp_drhoEA[0];
}

void
JunctionParallelChannels1PhaseUserObject::threadJoin(const UserObject & uo)
{
  VolumeJunctionBaseUserObject::threadJoin(uo);

  const JunctionParallelChannels1PhaseUserObject & jpc_uo =
      dynamic_cast<const JunctionParallelChannels1PhaseUserObject &>(uo);

  // Store the data computed/retrieved in the other threads
  for (unsigned int i = 0; i < jpc_uo._connection_indices.size(); i++)
  {
    const unsigned int c = jpc_uo._connection_indices[i];

    _areas[c] = jpc_uo._areas[c];
    _directions[c] = jpc_uo._directions[c];
    _is_inlet[c] = jpc_uo._is_inlet[c];

    _stored_pA[c] = jpc_uo._stored_pA[c];
    _stored_dp_drhoA[c] = jpc_uo._stored_dp_drhoA[c];
    _stored_dp_drhouA[c] = jpc_uo._stored_dp_drhouA[c];
    _stored_dp_drhoEA[c] = jpc_uo._stored_dp_drhoEA[c];

    if (c == 0)
      _d_flow = jpc_uo._d_flow;
  }
}

void
JunctionParallelChannels1PhaseUserObject::finalize()
{
  VolumeJunctionBaseUserObject::finalize();

  Real Ain_total = 0;
  Real pAin_total = 0;
  Real pAout_total = 0;
  Real Aout_total = 0;
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

  Real p_wall = 0;
  Real A_wall = 0;
  Point d_wall;
  bool out = false;
  bool p_non_zero = true;

  if (Aout_total > Ain_total)
  {
    out = true;
    if (Aout_total > 1e-15)
    {
      p_wall = pAout_total / Aout_total;
      _c_wall = _c_out;
    }
    else
    {
      p_wall = 0;
      p_non_zero = false;
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
      p_non_zero = false;
    }
    A_wall = Ain_total - Aout_total;
    d_wall = -_d_flow;
  }

  VolumeJunctionBaseUserObject::_residual[VolumeJunction1Phase::RHOUV_INDEX] -=
      d_wall(0) * p_wall * A_wall;
  VolumeJunctionBaseUserObject::_residual[VolumeJunction1Phase::RHOVV_INDEX] -=
      d_wall(1) * p_wall * A_wall;
  VolumeJunctionBaseUserObject::_residual[VolumeJunction1Phase::RHOWV_INDEX] -=
      d_wall(2) * p_wall * A_wall;

  if (p_non_zero == true)
  {
    std::vector<Real> dpwall_drhoA, dpwall_drhouA, dpwall_drhoEA;
    dpwall_drhoA.clear();
    dpwall_drhouA.clear();
    dpwall_drhoEA.clear();
    for (unsigned int ij = 0; ij < _c_wall.size(); ij++)
    {
      unsigned int c_ij = _c_wall[ij];
      if (out == true)
      {
        dpwall_drhoA.push_back(_stored_dp_drhoA[ij] * _areas[ij] / Aout_total);
        dpwall_drhouA.push_back(_stored_dp_drhouA[ij] * _areas[ij] / Aout_total);
        dpwall_drhoEA.push_back(_stored_dp_drhoEA[ij] * _areas[ij] / Aout_total);
      }
      else
      {
        dpwall_drhoA.push_back(_stored_dp_drhoA[ij] * _areas[ij] / Ain_total);
        dpwall_drhouA.push_back(_stored_dp_drhouA[ij] * _areas[ij] / Ain_total);
        dpwall_drhoEA.push_back(_stored_dp_drhoEA[ij] * _areas[ij] / Ain_total);
      }

      DenseMatrix<Real> jac(_n_scalar_eq, _n_flux_eq);
      for (unsigned int i = 1; i < (_n_scalar_eq - 1); i++)
      {
        // dpwall_drhoA
        jac(i, 0) = d_wall(i - 1) * dpwall_drhoA[ij] * A_wall;
        // dpwall_drhouA
        jac(i, 1) = d_wall(i - 1) * dpwall_drhouA[ij] * A_wall;
        // dpwall_drhoEA
        jac(i, 2) = d_wall(i - 1) * dpwall_drhoEA[ij] * A_wall;

        unsigned int jk = 0;
        for (unsigned int j = 0; j < _n_flux_eq; j++)
        {
          for (unsigned int k = 0; k < _phi_face_values[c_ij][j].size(); k++)
          {
            _residual_jacobian_flow_channel_vars[c_ij][i](0, jk) -=
                jac(i, j) * _phi_face_values[c_ij][j][k];
            jk++;
          }
        }
      }
    }
  }
}
