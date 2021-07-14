//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CNSFVHLLCBase.h"
#include "NS.h"
#include "HLLCUserObject.h"
#include "SinglePhaseFluidProperties.h"

namespace nms = NS;
using MetaPhysicL::raw_value;

InputParameters
CNSFVHLLCBase::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<UserObjectName>(nms::fluid, "Fluid properties userobject");
  return params;
}

CNSFVHLLCBase::CNSFVHLLCBase(const InputParameters & params)
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

HLLCData
CNSFVHLLCBase::hllcData() const
{
  return {_fluid,
          _rho_elem[_qp],
          _rho_neighbor[_qp],
          _vel_elem[_qp],
          _vel_neighbor[_qp],
          _specific_internal_energy_elem[_qp],
          _specific_internal_energy_neighbor[_qp]};
}

std::array<ADReal, 3>
CNSFVHLLCBase::waveSpeed(const HLLCData & hllc_data, const ADRealVectorValue & normal)
{
  const ADReal & rho1 = hllc_data.rho_elem;
  const ADReal u1 = hllc_data.vel_elem.norm();
  const ADReal q1 = normal * hllc_data.vel_elem;
  const ADReal v1 = 1.0 / rho1;
  const ADReal & e1 = hllc_data.e_elem;
  const ADReal E1 = e1 + 0.5 * u1 * u1;
  const ADReal p1 = hllc_data.fluid.p_from_v_e(v1, e1);
  const ADReal H1 = E1 + p1 / rho1;
  const ADReal c1 = hllc_data.fluid.c_from_v_e(v1, e1);

  const ADReal & rho2 = hllc_data.rho_neighbor;
  const ADReal u2 = hllc_data.vel_neighbor.norm();
  const ADReal q2 = normal * hllc_data.vel_neighbor;
  const ADReal v2 = 1.0 / rho2;
  const ADReal & e2 = hllc_data.e_neighbor;
  const ADReal E2 = e2 + 0.5 * u2 * u2;
  const ADReal p2 = hllc_data.fluid.p_from_v_e(v2, e2);
  const ADReal H2 = E2 + p2 / rho2;
  const ADReal c2 = hllc_data.fluid.c_from_v_e(v2, e2);

  // compute Roe-averaged variables
  const ADReal sqrt_rho1 = std::sqrt(rho1);
  const ADReal sqrt_rho2 = std::sqrt(rho2);
  const ADReal u_roe = (sqrt_rho1 * u1 + sqrt_rho2 * u2) / (sqrt_rho1 + sqrt_rho2);
  const ADReal q_roe = (sqrt_rho1 * q1 + sqrt_rho2 * q2) / (sqrt_rho1 + sqrt_rho2);
  const ADReal H_roe = (sqrt_rho1 * H1 + sqrt_rho2 * H2) / (sqrt_rho1 + sqrt_rho2);
  const ADReal h_roe = H_roe - 0.5 * u_roe * u_roe;
  const ADReal rho_roe = std::sqrt(rho1 * rho2);
  const ADReal v_roe = 1.0 / rho_roe;
  const ADReal e_roe = hllc_data.fluid.e_from_v_h(v_roe, h_roe);
  const ADReal c_roe = hllc_data.fluid.c_from_v_e(v_roe, e_roe);

  // compute wave speeds
  ADReal SL = std::min(q1 - c1, q_roe - c_roe);
  ADReal SR = std::max(q2 + c2, q_roe + c_roe);
  ADReal SM = (rho2 * q2 * (SR - q2) - rho1 * q1 * (SL - q1) + p1 - p2) /
              (rho2 * (SR - q2) - rho1 * (SL - q1));

  return {{std::move(SL), std::move(SM), std::move(SR)}};
}
