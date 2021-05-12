//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVKT.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Limiter.h"

using namespace Moose::FV;

registerMooseObject("MooseApp", PCNSFVKT);

InputParameters
PCNSFVKT::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Computes the residual of advective term using finite volume method.");
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid userobject");
  MooseEnum eqn("mass momentum energy scalar");
  params.addRequiredParam<MooseEnum>("eqn", eqn, "The equation you're solving.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this kernel applies to.");
  params.addParam<MaterialPropertyName>(
      "scalar_prop_name",
      "An optional material property name that can be used to specify an advected material "
      "property. If this is not supplied the variable variable will be used.");
  params.addParam<MooseEnum>(
      "limiter", moose_limiter_type, "The limiter to apply during interpolation.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PCNSFVKT::PCNSFVKT(const InputParameters & params)
  : FVFluxKernel(params),
    _fluid(getUserObject<SinglePhaseFluidProperties>(NS::fluid)),

    _sup_vel_x_elem(getADMaterialProperty<Real>(NS::superficial_velocity_x)),
    _sup_vel_x_neighbor(getNeighborADMaterialProperty<Real>(NS::superficial_velocity_x)),
    _grad_sup_vel_x_elem(
        getADMaterialProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_x))),
    _grad_sup_vel_x_neighbor(
        getNeighborADMaterialProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_x))),
    _sup_vel_y_elem(getADMaterialProperty<Real>(NS::superficial_velocity_y)),
    _sup_vel_y_neighbor(getNeighborADMaterialProperty<Real>(NS::superficial_velocity_y)),
    _grad_sup_vel_y_elem(
        getADMaterialProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_y))),
    _grad_sup_vel_y_neighbor(
        getNeighborADMaterialProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_y))),
    _sup_vel_z_elem(getADMaterialProperty<Real>(NS::superficial_velocity_z)),
    _sup_vel_z_neighbor(getNeighborADMaterialProperty<Real>(NS::superficial_velocity_z)),
    _grad_sup_vel_z_elem(
        getADMaterialProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_z))),
    _grad_sup_vel_z_neighbor(
        getNeighborADMaterialProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_z))),

    _T_fluid_elem(getADMaterialProperty<Real>(NS::T_fluid)),
    _T_fluid_neighbor(getNeighborADMaterialProperty<Real>(NS::T_fluid)),
    _grad_T_fluid_elem(getADMaterialProperty<RealVectorValue>(NS::grad(NS::T_fluid))),
    _grad_T_fluid_neighbor(getNeighborADMaterialProperty<RealVectorValue>(NS::grad(NS::T_fluid))),
    _pressure_elem(getADMaterialProperty<Real>(NS::pressure)),
    _pressure_neighbor(getNeighborADMaterialProperty<Real>(NS::pressure)),
    _grad_pressure_elem(getADMaterialProperty<RealVectorValue>(NS::grad(NS::pressure))),
    _grad_pressure_neighbor(getNeighborADMaterialProperty<RealVectorValue>(NS::grad(NS::pressure))),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity)),
    _eqn(getParam<MooseEnum>("eqn")),
    _index(getParam<MooseEnum>("momentum_component")),
    _scalar_elem(isParamValid("scalar_prop_name")
                     ? getADMaterialProperty<Real>("scalar_prop_name").get()
                     : _u_elem),
    _scalar_neighbor(isParamValid("scalar_prop_name")
                         ? getNeighborADMaterialProperty<Real>("scalar_prop_name").get()
                         : _u_neighbor),
    _limiter(Limiter::build(LimiterType(int(getParam<MooseEnum>("limiter")))))
{
  if ((_eqn == "momentum") && !isParamValid("momentum_component"))
    paramError("eqn",
               "If 'momentum' is specified for 'eqn', then you must provide a parameter "
               "value for 'momentum_component'");
}

ADReal
PCNSFVKT::computeQpResidual()
{
  // Perform primitive interpolations
  const auto pressure_elem = interpolate(*_limiter,
                                         _pressure_elem[_qp],
                                         _pressure_neighbor[_qp],
                                         _grad_pressure_elem[_qp],
                                         *_face_info,
                                         /*elem_is_up=*/true);
  const auto pressure_neighbor = interpolate(*_limiter,
                                             _pressure_neighbor[_qp],
                                             _pressure_elem[_qp],
                                             _grad_pressure_neighbor[_qp],
                                             *_face_info,
                                             /*elem_is_up=*/false);
  const auto T_fluid_elem = interpolate(*_limiter,
                                        _T_fluid_elem[_qp],
                                        _T_fluid_neighbor[_qp],
                                        _grad_T_fluid_elem[_qp],
                                        *_face_info,
                                        /*elem_is_up=*/true);
  const auto T_fluid_neighbor = interpolate(*_limiter,
                                            _T_fluid_neighbor[_qp],
                                            _T_fluid_elem[_qp],
                                            _grad_T_fluid_neighbor[_qp],
                                            *_face_info,
                                            /*elem_is_up=*/false);
  const auto sup_vel_x_elem = interpolate(*_limiter,
                                          _sup_vel_x_elem[_qp],
                                          _sup_vel_x_neighbor[_qp],
                                          _grad_sup_vel_x_elem[_qp],
                                          *_face_info,
                                          /*elem_is_up=*/true);
  const auto sup_vel_x_neighbor = interpolate(*_limiter,
                                              _sup_vel_x_neighbor[_qp],
                                              _sup_vel_x_elem[_qp],
                                              _grad_sup_vel_x_neighbor[_qp],
                                              *_face_info,
                                              /*elem_is_up=*/false);
  const auto sup_vel_y_elem = interpolate(*_limiter,
                                          _sup_vel_y_elem[_qp],
                                          _sup_vel_y_neighbor[_qp],
                                          _grad_sup_vel_y_elem[_qp],
                                          *_face_info,
                                          /*elem_is_up=*/true);
  const auto sup_vel_y_neighbor = interpolate(*_limiter,
                                              _sup_vel_y_neighbor[_qp],
                                              _sup_vel_y_elem[_qp],
                                              _grad_sup_vel_y_neighbor[_qp],
                                              *_face_info,
                                              /*elem_is_up=*/false);
  const auto sup_vel_z_elem = interpolate(*_limiter,
                                          _sup_vel_z_elem[_qp],
                                          _sup_vel_z_neighbor[_qp],
                                          _grad_sup_vel_z_elem[_qp],
                                          *_face_info,
                                          /*elem_is_up=*/true);
  const auto sup_vel_z_neighbor = interpolate(*_limiter,
                                              _sup_vel_z_neighbor[_qp],
                                              _sup_vel_z_elem[_qp],
                                              _grad_sup_vel_z_neighbor[_qp],
                                              *_face_info,
                                              /*elem_is_up=*/false);

  const auto sup_vel_elem = VectorValue<ADReal>(sup_vel_x_elem, sup_vel_y_elem, sup_vel_z_elem);
  const auto u_elem = sup_vel_elem / _eps_elem[_qp];
  const auto rho_elem = _fluid.rho_from_p_T(pressure_elem, T_fluid_elem);
  const auto e_elem = _fluid.e_from_p_rho(pressure_elem, rho_elem);
  const auto specific_volume_elem = 1. / rho_elem;
  const auto sup_vel_neighbor =
      VectorValue<ADReal>(sup_vel_x_neighbor, sup_vel_y_neighbor, sup_vel_z_neighbor);
  const auto u_neighbor = sup_vel_neighbor / _eps_neighbor[_qp];
  const auto rho_neighbor = _fluid.rho_from_p_T(pressure_neighbor, T_fluid_neighbor);
  const auto e_neighbor = _fluid.e_from_p_rho(pressure_neighbor, rho_neighbor);
  const auto specific_volume_neighbor = 1. / rho_neighbor;

  const auto c_elem = _fluid.c_from_v_e(specific_volume_elem, e_elem);
  const auto c_neighbor = _fluid.c_from_v_e(specific_volume_neighbor, e_neighbor);

  const auto sup_vel_elem_normal = sup_vel_elem * _face_info->normal();
  const auto sup_vel_neighbor_normal = sup_vel_neighbor * _face_info->normal();
  const auto u_elem_normal = u_elem * _face_info->normal();
  const auto u_neighbor_normal = u_neighbor * _face_info->normal();

  const auto a_elem = std::max(std::abs(u_elem_normal + c_elem), std::abs(u_elem_normal - c_elem));
  const auto a_neighbor =
      std::max(std::abs(u_neighbor_normal + c_neighbor), std::abs(u_neighbor_normal - c_neighbor));
  // Second term is to avoid new nonzero mallocs
  const auto a = std::max(a_elem, a_neighbor) + 0 * a_elem + 0 * a_neighbor;

  if (_eqn == "mass")
    return 0.5 * (sup_vel_elem_normal * rho_elem + sup_vel_neighbor_normal * rho_neighbor -
                  a * (rho_neighbor - rho_elem));
  else if (_eqn == "momentum")
  {
    const auto rhou_elem = u_elem(_index) * rho_elem;
    const auto rhou_neighbor = u_neighbor(_index) * rho_neighbor;
    return 0.5 * (sup_vel_elem_normal * rhou_elem + sup_vel_neighbor_normal * rhou_neighbor +
                  (_eps_elem[_qp] * pressure_elem + _eps_neighbor[_qp] * pressure_neighbor) *
                      _face_info->normal()(_index) -
                  a * (rhou_neighbor - rhou_elem));
  }
  else if (_eqn == "energy")
  {
    const auto ht_elem = e_elem + 0.5 * u_elem * u_elem + pressure_elem / rho_elem;
    const auto ht_neighbor =
        e_neighbor + 0.5 * u_neighbor * u_neighbor + pressure_neighbor / rho_neighbor;
    const auto rho_ht_elem = rho_elem * ht_elem;
    const auto rho_ht_neighbor = rho_neighbor * ht_neighbor;
    return 0.5 * (sup_vel_elem_normal * rho_ht_elem + sup_vel_neighbor_normal * rho_ht_neighbor -
                  a * (rho_ht_neighbor - rho_ht_elem));
  }
  else if (_eqn == "scalar")
    return sup_vel_elem_normal * rho_elem * _scalar_elem[_qp] +
           sup_vel_neighbor_normal * rho_neighbor * _scalar_neighbor[_qp];
  else
    mooseError("Unrecognized enum type ", _eqn);
}
