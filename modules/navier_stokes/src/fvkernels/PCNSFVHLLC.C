//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVHLLC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"

using MetaPhysicL::raw_value;

InputParameters
PCNSFVHLLC::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid properties userobject");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PCNSFVHLLC::PCNSFVHLLC(const InputParameters & params)
  : FVFluxKernel(params),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _specific_internal_energy_elem(getADMaterialProperty<Real>(NS::specific_internal_energy)),
    _specific_internal_energy_neighbor(
        getNeighborADMaterialProperty<Real>(NS::specific_internal_energy)),
    _rho_et_elem(getADMaterialProperty<Real>(NS::total_energy_density)),
    _rho_et_neighbor(getNeighborADMaterialProperty<Real>(NS::total_energy_density)),
    _vel_elem(getADMaterialProperty<RealVectorValue>(NS::velocity)),
    _vel_neighbor(getNeighborADMaterialProperty<RealVectorValue>(NS::velocity)),
    _speed_elem(getADMaterialProperty<Real>(NS::speed)),
    _speed_neighbor(getNeighborADMaterialProperty<Real>(NS::speed)),
    _rho_elem(getADMaterialProperty<Real>(NS::density)),
    _rho_neighbor(getNeighborADMaterialProperty<Real>(NS::density)),
    _pressure_elem(getADMaterialProperty<Real>(NS::pressure)),
    _pressure_neighbor(getNeighborADMaterialProperty<Real>(NS::pressure)),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity))
{
}

std::array<ADReal, 3>
PCNSFVHLLC::waveSpeed(const ADReal & rho_elem,
                      const ADRealVectorValue & vel_elem,
                      const ADReal & e_elem,
                      const Real eps_elem,
                      const ADReal & rho_neighbor,
                      const ADRealVectorValue & vel_neighbor,
                      const ADReal & e_neighbor,
                      const Real eps_neighbor,
                      const SinglePhaseFluidProperties & fluid,
                      const ADRealVectorValue & normal)
{
  const auto & rho1 = rho_elem;
  const auto u1 = vel_elem.norm();
  const auto q1 = normal * vel_elem;
  const auto v1 = 1.0 / rho1;
  const auto & e1 = e_elem;
  const auto et1 = e1 + 0.5 * u1 * u1;
  const auto p1 = fluid.p_from_v_e(v1, e1);
  const auto ht1 = et1 + p1 / rho1;
  const auto c1 = fluid.c_from_v_e(v1, e1);
  const auto eps1 = eps_elem;

  const auto & rho2 = rho_neighbor;
  const auto u2 = vel_neighbor.norm();
  const auto q2 = normal * vel_neighbor;
  const auto v2 = 1.0 / rho2;
  const auto & e2 = e_neighbor;
  const auto et2 = e2 + 0.5 * u2 * u2;
  const auto p2 = fluid.p_from_v_e(v2, e2);
  const auto ht2 = et2 + p2 / rho2;
  const auto c2 = fluid.c_from_v_e(v2, e2);
  const auto eps2 = eps_neighbor;

  // compute Roe-averaged variables
  const auto sqrt_rho1 = std::sqrt(rho1);
  const auto sqrt_rho2 = std::sqrt(rho2);
  const auto u_roe = (sqrt_rho1 * u1 + sqrt_rho2 * u2) / (sqrt_rho1 + sqrt_rho2);
  const auto q_roe = (sqrt_rho1 * q1 + sqrt_rho2 * q2) / (sqrt_rho1 + sqrt_rho2);
  const auto e_roe = (sqrt_rho1 * e1 + sqrt_rho2 * e2) / (sqrt_rho1 + sqrt_rho2);
  const auto rho_roe = std::sqrt(rho1 * rho2);
  const auto v_roe = 1.0 / rho_roe;
  const auto c_roe = fluid.c_from_v_e(v_roe, e_roe);

  // compute wave speeds
  // I may want to change the estimate of these wave speeds!
  auto SL = std::min(q1 - c1, q_roe - c_roe);
  auto SR = std::max(q2 + c2, q_roe + c_roe);
  auto SM = (eps2 * rho2 * q2 * (SR - q2) - eps1 * rho1 * q1 * (SL - q1) + eps1 * p1 - eps2 * p2) /
            (eps2 * rho2 * (SR - q2) - eps1 * rho1 * (SL - q1));

  return {{std::move(SL), std::move(SM), std::move(SR)}};
}

ADReal
PCNSFVHLLC::computeQpResidual()
{
  _normal_speed_elem = _normal * _vel_elem[_qp];
  _normal_speed_neighbor = _normal * _vel_neighbor[_qp];
  auto wave_speeds = waveSpeed(_rho_elem[_qp],
                               _vel_elem[_qp],
                               _specific_internal_energy_elem[_qp],
                               _eps_elem[_qp],
                               _rho_neighbor[_qp],
                               _vel_neighbor[_qp],
                               _specific_internal_energy_neighbor[_qp],
                               _eps_neighbor[_qp],
                               _fluid,
                               _normal);
  _SL = std::move(wave_speeds[0]);
  _SM = std::move(wave_speeds[1]);
  _SR = std::move(wave_speeds[2]);
  if (_SL >= 0)
    return fluxElem();
  else if (_SR <= 0)
    return fluxNeighbor();
  else
  {
    if (_SM >= 0)
    {
      ADReal f = _eps_elem[_qp] * _rho_elem[_qp] * (_SL - _normal_speed_elem) / (_SL - _SM);
      return fluxElem() + _SL * (f * hllcElem() - conservedVariableElem());
    }
    else
    {
      ADReal f =
          _eps_neighbor[_qp] * _rho_neighbor[_qp] * (_SR - _normal_speed_neighbor) / (_SR - _SM);
      return fluxNeighbor() + _SR * (f * hllcNeighbor() - conservedVariableNeighbor());
    }
  }
  mooseError("Should never get here");
}
