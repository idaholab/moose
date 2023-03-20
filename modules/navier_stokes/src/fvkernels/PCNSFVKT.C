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

registerMooseObject("NavierStokesApp", PCNSFVKT);

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
  params.addParam<MooseEnum>(
      "limiter", moose_limiter_type, "The limiter to apply during interpolation.");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addParam<bool>(
      "knp_for_omega",
      true,
      "Whether to use the Kurganov, Noelle, and Petrova method to compute the omega parameter for "
      "stabilization. If false, then the Kurganov-Tadmor method will be used.");
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
    _scalar_elem(_u_elem),
    _scalar_neighbor(_u_neighbor),
    _grad_scalar_elem((_eqn == "scalar") ? &_var.adGradSln() : nullptr),
    _grad_scalar_neighbor((_eqn == "scalar") ? &_var.adGradSlnNeighbor() : nullptr),
    _limiter(Limiter<ADReal>::build(LimiterType(int(getParam<MooseEnum>("limiter"))))),
    _knp_for_omega(getParam<bool>("knp_for_omega"))
{
  if ((_eqn == "momentum") && !isParamValid("momentum_component"))
    paramError("eqn",
               "If 'momentum' is specified for 'eqn', then you must provide a parameter "
               "value for 'momentum_component'");
}

std::pair<ADReal, ADReal>
PCNSFVKT::computeAlphaAndOmega(const ADReal & u_elem_normal,
                               const ADReal & u_neighbor_normal,
                               const ADReal & c_elem,
                               const ADReal & c_neighbor) const
{
  // Equations from Greenshields sans multiplication by area (which will be done in
  // computeResidual/Jacobian
  const auto psi_elem =
      std::max({c_elem + u_elem_normal, c_neighbor + u_neighbor_normal, ADReal(0)});
  const auto psi_neighbor =
      std::max({c_elem - u_elem_normal, c_neighbor - u_neighbor_normal, ADReal(0)});
  auto alpha = _knp_for_omega ? psi_elem / (psi_elem + psi_neighbor) : ADReal(0.5);
  auto omega = _knp_for_omega ? alpha * (1 - alpha) * (psi_elem + psi_neighbor)
                              : alpha * std::max(psi_elem, psi_neighbor);

  // Do this to avoid new nonzero mallocs
  const auto dummy_quant = 0 * (c_elem + u_elem_normal + c_neighbor + u_neighbor_normal);

  alpha += dummy_quant;
  omega += dummy_quant;
  return std::make_pair(std::move(alpha), std::move(omega));
}

ADReal
PCNSFVKT::computeFaceFlux(const ADReal & alpha,
                          const ADReal & omega,
                          const ADReal & sup_vel_elem_normal,
                          const ADReal & sup_vel_neighbor_normal,
                          const ADReal & adv_quant_elem,
                          const ADReal & adv_quant_neighbor)
{
  return alpha * (sup_vel_elem_normal * adv_quant_elem) +
         (1 - alpha) * sup_vel_neighbor_normal * adv_quant_neighbor -
         omega * (adv_quant_neighbor - adv_quant_elem);
}

ADReal
PCNSFVKT::computeQpResidual()
{
  // Perform primitive interpolations
  const auto pressure_elem = interpolate(*_limiter,
                                         _pressure_elem[_qp],
                                         _pressure_neighbor[_qp],
                                         &_grad_pressure_elem[_qp],
                                         *_face_info,
                                         /*elem_is_up=*/true);
  const auto pressure_neighbor = interpolate(*_limiter,
                                             _pressure_neighbor[_qp],
                                             _pressure_elem[_qp],
                                             &_grad_pressure_neighbor[_qp],
                                             *_face_info,
                                             /*elem_is_up=*/false);
  const auto T_fluid_elem = interpolate(*_limiter,
                                        _T_fluid_elem[_qp],
                                        _T_fluid_neighbor[_qp],
                                        &_grad_T_fluid_elem[_qp],
                                        *_face_info,
                                        /*elem_is_up=*/true);
  const auto T_fluid_neighbor = interpolate(*_limiter,
                                            _T_fluid_neighbor[_qp],
                                            _T_fluid_elem[_qp],
                                            &_grad_T_fluid_neighbor[_qp],
                                            *_face_info,
                                            /*elem_is_up=*/false);
  const auto sup_vel_x_elem = interpolate(*_limiter,
                                          _sup_vel_x_elem[_qp],
                                          _sup_vel_x_neighbor[_qp],
                                          &_grad_sup_vel_x_elem[_qp],
                                          *_face_info,
                                          /*elem_is_up=*/true);
  const auto sup_vel_x_neighbor = interpolate(*_limiter,
                                              _sup_vel_x_neighbor[_qp],
                                              _sup_vel_x_elem[_qp],
                                              &_grad_sup_vel_x_neighbor[_qp],
                                              *_face_info,
                                              /*elem_is_up=*/false);
  const auto sup_vel_y_elem = interpolate(*_limiter,
                                          _sup_vel_y_elem[_qp],
                                          _sup_vel_y_neighbor[_qp],
                                          &_grad_sup_vel_y_elem[_qp],
                                          *_face_info,
                                          /*elem_is_up=*/true);
  const auto sup_vel_y_neighbor = interpolate(*_limiter,
                                              _sup_vel_y_neighbor[_qp],
                                              _sup_vel_y_elem[_qp],
                                              &_grad_sup_vel_y_neighbor[_qp],
                                              *_face_info,
                                              /*elem_is_up=*/false);
  const auto sup_vel_z_elem = interpolate(*_limiter,
                                          _sup_vel_z_elem[_qp],
                                          _sup_vel_z_neighbor[_qp],
                                          &_grad_sup_vel_z_elem[_qp],
                                          *_face_info,
                                          /*elem_is_up=*/true);
  const auto sup_vel_z_neighbor = interpolate(*_limiter,
                                              _sup_vel_z_neighbor[_qp],
                                              _sup_vel_z_elem[_qp],
                                              &_grad_sup_vel_z_neighbor[_qp],
                                              *_face_info,
                                              /*elem_is_up=*/false);

  const auto sup_vel_elem = VectorValue<ADReal>(sup_vel_x_elem, sup_vel_y_elem, sup_vel_z_elem);
  const auto u_elem = sup_vel_elem / _eps_elem[_qp];
  const auto rho_elem = _fluid.rho_from_p_T(pressure_elem, T_fluid_elem);
  const auto specific_volume_elem = 1. / rho_elem;
  const auto e_elem = _fluid.e_from_T_v(T_fluid_elem, specific_volume_elem);
  const auto sup_vel_neighbor =
      VectorValue<ADReal>(sup_vel_x_neighbor, sup_vel_y_neighbor, sup_vel_z_neighbor);
  const auto u_neighbor = sup_vel_neighbor / _eps_neighbor[_qp];
  const auto rho_neighbor = _fluid.rho_from_p_T(pressure_neighbor, T_fluid_neighbor);
  const auto specific_volume_neighbor = 1. / rho_neighbor;
  const auto e_neighbor = _fluid.e_from_T_v(T_fluid_neighbor, specific_volume_neighbor);

  const auto c_elem = _fluid.c_from_v_e(specific_volume_elem, e_elem);
  const auto c_neighbor = _fluid.c_from_v_e(specific_volume_neighbor, e_neighbor);

  const auto sup_vel_elem_normal = sup_vel_elem * _face_info->normal();
  const auto sup_vel_neighbor_normal = sup_vel_neighbor * _face_info->normal();
  const auto u_elem_normal = u_elem * _face_info->normal();
  const auto u_neighbor_normal = u_neighbor * _face_info->normal();

  const auto pr = computeAlphaAndOmega(u_elem_normal, u_neighbor_normal, c_elem, c_neighbor);
  const auto & alpha = pr.first;
  const auto & omega = pr.second;

  if (_eqn == "mass")
    return computeFaceFlux(
        alpha, omega, sup_vel_elem_normal, sup_vel_neighbor_normal, rho_elem, rho_neighbor);
  else if (_eqn == "momentum")
  {
    const auto rhou_elem = u_elem(_index) * rho_elem;
    const auto rhou_neighbor = u_neighbor(_index) * rho_neighbor;
    return computeFaceFlux(alpha,
                           omega,
                           sup_vel_elem_normal,
                           sup_vel_neighbor_normal,
                           rhou_elem,
                           rhou_neighbor) +
           _face_info->normal()(_index) * (alpha * _eps_elem[_qp] * pressure_elem +
                                           (1 - alpha) * _eps_neighbor[_qp] * pressure_neighbor);
  }
  else if (_eqn == "energy")
  {
    const auto ht_elem = e_elem + 0.5 * u_elem * u_elem + pressure_elem / rho_elem;
    const auto ht_neighbor =
        e_neighbor + 0.5 * u_neighbor * u_neighbor + pressure_neighbor / rho_neighbor;
    const auto rho_ht_elem = rho_elem * ht_elem;
    const auto rho_ht_neighbor = rho_neighbor * ht_neighbor;
    return computeFaceFlux(
        alpha, omega, sup_vel_elem_normal, sup_vel_neighbor_normal, rho_ht_elem, rho_ht_neighbor);
  }
  else if (_eqn == "scalar")
  {
    const auto scalar_elem = interpolate(*_limiter,
                                         _scalar_elem[_qp],
                                         _scalar_neighbor[_qp],
                                         &(*_grad_scalar_elem)[_qp],
                                         *_face_info,
                                         true);
    const auto scalar_neighbor = interpolate(*_limiter,
                                             _scalar_neighbor[_qp],
                                             _scalar_elem[_qp],
                                             &(*_grad_scalar_neighbor)[_qp],
                                             *_face_info,
                                             false);
    const auto rhos_elem = rho_elem * scalar_elem;
    const auto rhos_neighbor = rho_neighbor * scalar_neighbor;
    return computeFaceFlux(
        alpha, omega, sup_vel_elem_normal, sup_vel_neighbor_normal, rhos_elem, rhos_neighbor);
  }
  else
    mooseError("Unrecognized enum type ", _eqn);
}
