//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HLLCUserObject.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"

using MetaPhysicL::raw_value;

registerMooseObject("NavierStokesApp", HLLCUserObject);

InputParameters
HLLCUserObject::validParams()
{
  InputParameters params = InternalSideUserObject::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid properties userobject");
  params.addParam<MaterialPropertyName>(
      NS::porosity,
      "An optional parameter for specifying a porosity material property. If not specified "
      "free-flow conditions will be assumed.");
  params.addClassDescription(
      "Computes free-flow wave speeds on internal sides, useful in HLLC contexts");
  return params;
}

HLLCUserObject::HLLCUserObject(const InputParameters & parameters)
  : InternalSideUserObject(parameters),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _face_info(_mesh.faceInfo()),
    _vel_elem(getADMaterialProperty<RealVectorValue>(NS::velocity)),
    _vel_neighbor(getNeighborADMaterialProperty<RealVectorValue>(NS::velocity)),
    _speed_elem(getADMaterialProperty<Real>(NS::speed)),
    _speed_neighbor(getNeighborADMaterialProperty<Real>(NS::speed)),
    _pressure_elem(getADMaterialProperty<Real>(NS::pressure)),
    _pressure_neighbor(getNeighborADMaterialProperty<Real>(NS::pressure)),
    _rho_elem(getADMaterialProperty<Real>(NS::density)),
    _rho_neighbor(getNeighborADMaterialProperty<Real>(NS::density)),
    _specific_internal_energy_elem(getADMaterialProperty<Real>(NS::specific_internal_energy)),
    _specific_internal_energy_neighbor(
        getNeighborADMaterialProperty<Real>(NS::specific_internal_energy)),
    _eps_elem(isParamValid(NS::porosity) ? &getMaterialProperty<Real>(NS::porosity) : nullptr),
    _eps_neighbor(isParamValid(NS::porosity) ? &getNeighborMaterialProperty<Real>(NS::porosity)
                                             : nullptr)
{
}

void
HLLCUserObject::initialSetup()
{
  for (unsigned int j = 0; j < _face_info.size(); ++j)
  {
    const Elem * elem = &_face_info[j]->elem();
    unsigned int side = _face_info[j]->elemSideID();
    side_type elem_side(elem, side);
    _side_to_face_info[elem_side] = j;
  }
}

void
HLLCUserObject::execute()
{
  const FaceInfo & fi = faceInfoHelper(_current_elem, _current_side);

  const ADReal & rho1 = _rho_elem[_qp];
  const ADReal & u1 = _speed_elem[_qp];
  const ADReal q1 = fi.normal() * _vel_elem[_qp];
  const ADReal v1 = 1.0 / rho1;
  const ADReal & e1 = _specific_internal_energy_elem[_qp];
  const ADReal E1 = e1 + 0.5 * u1 * u1;
  const ADReal & p1 = _pressure_elem[_qp];
  const ADReal H1 = E1 + p1 / rho1;
  const ADReal c1 = _fluid.c_from_v_e(v1, e1);
  const Real eps1 = _eps_elem ? (*_eps_elem)[_qp] : 1.;

  const ADReal & rho2 = _rho_neighbor[_qp];
  const ADReal & u2 = _speed_neighbor[_qp];
  const ADReal q2 = fi.normal() * _vel_neighbor[_qp];
  const ADReal v2 = 1.0 / rho2;
  const ADReal & e2 = _specific_internal_energy_neighbor[_qp];
  const ADReal E2 = e2 + 0.5 * u2 * u2;
  const ADReal & p2 = _pressure_neighbor[_qp];
  const ADReal H2 = E2 + p2 / rho2;
  const ADReal c2 = _fluid.c_from_v_e(v2, e2);
  const Real eps2 = _eps_neighbor ? (*_eps_neighbor)[_qp] : 1.;

  // compute Roe-averaged variables
  const ADReal sqrt_rho1 = std::sqrt(rho1);
  const ADReal sqrt_rho2 = std::sqrt(rho2);
  const ADReal u_roe = (sqrt_rho1 * u1 + sqrt_rho2 * u2) / (sqrt_rho1 + sqrt_rho2);
  const ADReal q_roe = (sqrt_rho1 * q1 + sqrt_rho2 * q2) / (sqrt_rho1 + sqrt_rho2);
  const ADReal H_roe = (sqrt_rho1 * H1 + sqrt_rho2 * H2) / (sqrt_rho1 + sqrt_rho2);
  const ADReal h_roe = H_roe - 0.5 * u_roe * u_roe;
  const ADReal rho_roe = std::sqrt(rho1 * rho2);
  const ADReal v_roe = 1.0 / rho_roe;
  const ADReal e_roe = _fluid.e_from_v_h(v_roe, h_roe);
  const ADReal c_roe = _fluid.c_from_v_e(v_roe, e_roe);

  // compute wave speeds
  const ADReal SL = std::min(q1 - c1, q_roe - c_roe);
  const ADReal SR = std::max(q2 + c2, q_roe + c_roe);
  const ADReal SM =
      (eps2 * rho2 * q2 * (SR - q2) - eps1 * rho1 * q1 * (SL - q1) + eps1 * p1 - eps2 * p2) /
      (eps2 * rho2 * (SR - q2) - eps1 * rho1 * (SL - q1));

  // store these results in _wave_speed
  side_type elem_side(_current_elem, _current_side);
  _wave_speed[elem_side] = {SL, SM, SR};
}

bool
HLLCUserObject::hasData(const Elem * const elem, const unsigned int side) const
{
  side_type elem_side(elem, side);
  return (_wave_speed.find(elem_side) != _wave_speed.end());
}

void
HLLCUserObject::threadJoin(const UserObject & y)
{
  const HLLCUserObject & pps = static_cast<const HLLCUserObject &>(y);
  for (auto & ws : pps._wave_speed)
  {
    const auto & it = _wave_speed.find(ws.first);
    if (it == _wave_speed.end())
      _wave_speed[ws.first] = ws.second;
    else
      for (unsigned int j = 0; j < 3; ++j)
        _wave_speed[ws.first][j] = ws.second[j];
  }
}

std::vector<ADReal>
HLLCUserObject::waveSpeed(const Elem * elem, unsigned int side) const
{
  side_type elem_side(elem, side);
  auto it = _wave_speed.find(elem_side);
  if (it == _wave_speed.end())
    mooseError("what the heck (version 2)?");
  return it->second;
}

const FaceInfo &
HLLCUserObject::faceInfoHelper(const Elem * elem, unsigned int side) const
{
  side_type elem_side(elem, side);
  auto it = _side_to_face_info.find(elem_side);
  if (it == _side_to_face_info.end())
    mooseError("what the heck?");
  return *_face_info[it->second];
}
