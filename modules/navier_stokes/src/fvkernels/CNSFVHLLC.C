//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLC.h"
#include "NS.h"
#include "HLLCUserObject.h"
#include "SinglePhaseFluidProperties.h"

namespace nms = NS;
using MetaPhysicL::raw_value;

InputParameters
CNSFVHLLC::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<UserObjectName>(nms::fluid, "Fluid userobject");
  return params;
}

CNSFVHLLC::CNSFVHLLC(const InputParameters & params)
  : FVFluxKernel(params),
    _fluid(dynamic_cast<FEProblemBase *>(&_subproblem)
               ->getUserObject<SinglePhaseFluidProperties>(nms::fluid)),
    _specific_internal_energy_elem(getADMaterialProperty<Real>(nms::specific_internal_energy)),
    _specific_internal_energy_neighbor(
        getNeighborADMaterialProperty<Real>(nms::specific_internal_energy)),
    _rho_et_elem(getADMaterialProperty<Real>(nms::total_energy_density)),
    _rho_et_neighbor(getNeighborADMaterialProperty<Real>(nms::total_energy_density)),
    _vel_elem(getADMaterialProperty<RealVectorValue>(nms::velocity)),
    _vel_neighbor(getNeighborADMaterialProperty<RealVectorValue>(nms::velocity)),
    _speed_elem(getADMaterialProperty<Real>(nms::speed)),
    _speed_neighbor(getNeighborADMaterialProperty<Real>(nms::speed)),
    _rho_elem(getADMaterialProperty<Real>(nms::density)),
    _rho_neighbor(getNeighborADMaterialProperty<Real>(nms::density)),
    _pressure_elem(getADMaterialProperty<Real>(nms::pressure)),
    _pressure_neighbor(getNeighborADMaterialProperty<Real>(nms::pressure))
{
}

std::vector<ADReal>
CNSFVHLLC::waveSpeed(const ADReal & rho_elem,
                     const ADRealVectorValue & vel_elem,
                     const ADReal & e_elem,
                     const ADReal & rho_neighbor,
                     const ADRealVectorValue & vel_neighbor,
                     const ADReal & e_neighbor,
                     const SinglePhaseFluidProperties & fluid,
                     const ADRealVectorValue & normal)
{
  const ADReal rho1 = rho_elem;
  const ADReal u1 = vel_elem.norm();
  const ADReal q1 = normal * vel_elem;
  const ADReal v1 = 1.0 / rho1;
  const ADReal e1 = e_elem;
  const ADReal E1 = e1 + 0.5 * u1 * u1;
  const ADReal p1 = fluid.p_from_v_e(v1, e1);
  const ADReal H1 = E1 + p1 / rho1;
  const ADReal c1 = fluid.c_from_v_e(v1, e1);

  const ADReal rho2 = rho_neighbor;
  const ADReal u2 = vel_neighbor.norm();
  const ADReal q2 = normal * vel_neighbor;
  const ADReal v2 = 1.0 / rho2;
  const ADReal e2 = e_neighbor;
  const ADReal E2 = e2 + 0.5 * u2 * u2;
  const ADReal p2 = fluid.p_from_v_e(v2, e2);
  const ADReal H2 = E2 + p2 / rho2;
  const ADReal c2 = fluid.c_from_v_e(v2, e2);

  // compute Roe-averaged variables
  const ADReal sqrt_rho1 = std::sqrt(rho1);
  const ADReal sqrt_rho2 = std::sqrt(rho2);
  const ADReal u_roe = (sqrt_rho1 * u1 + sqrt_rho2 * u2) / (sqrt_rho1 + sqrt_rho2);
  const ADReal q_roe = (sqrt_rho1 * q1 + sqrt_rho2 * q2) / (sqrt_rho1 + sqrt_rho2);
  const ADReal H_roe = (sqrt_rho1 * H1 + sqrt_rho2 * H2) / (sqrt_rho1 + sqrt_rho2);
  const ADReal h_roe = H_roe - 0.5 * u_roe * u_roe;
  const ADReal rho_roe = std::sqrt(rho1 * rho2);
  const ADReal v_roe = 1.0 / rho_roe;
  const ADReal e_roe = fluid.e_from_v_h(v_roe, h_roe);
  const ADReal c_roe = fluid.c_from_v_e(v_roe, e_roe);

  // compute wave speeds
  const ADReal SL = std::min(q1 - c1, q_roe - c_roe);
  const ADReal SR = std::max(q2 + c2, q_roe + c_roe);
  const ADReal SM = (rho2 * q2 * (SR - q2) - rho1 * q1 * (SL - q1) + p1 - p2) /
                    (rho2 * (SR - q2) - rho1 * (SL - q1));

  // store these results in _wave_speed
  return {SL, SM, SR};
}

ADReal
CNSFVHLLC::computeQpResidual()
{
  _normal_speed_elem = _normal * _vel_elem[_qp];
  _normal_speed_neighbor = _normal * _vel_neighbor[_qp];
  const auto & wave_speeds = waveSpeed(_rho_elem[_qp],
                                       _vel_elem[_qp],
                                       _specific_internal_energy_elem[_qp],
                                       _rho_neighbor[_qp],
                                       _vel_neighbor[_qp],
                                       _specific_internal_energy_neighbor[_qp],
                                       _fluid,
                                       _normal);
  _SL = wave_speeds[0];
  _SM = wave_speeds[1];
  _SR = wave_speeds[2];
  if (_SL >= 0)
    return fluxElem();
  else if (_SR <= 0)
    return fluxNeighbor();
  else
  {
    if (_SM >= 0)
    {
      ADReal f = _rho_elem[_qp] * (_SL - _normal_speed_elem) / (_SL - _SM);
      return fluxElem() + _SL * (f * hllcElem() - conservedVariableElem());
    }
    else
    {
      ADReal f = _rho_neighbor[_qp] * (_SR - _normal_speed_neighbor) / (_SR - _SM);
      return fluxNeighbor() + _SR * (f * hllcNeighbor() - conservedVariableNeighbor());
    }
  }
  mooseError("Should never get here");
}
