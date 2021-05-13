//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVKTBC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"
#include "Limiter.h"

using namespace Moose::FV;

registerMooseObject("MooseApp", PCNSFVKTBC);

InputParameters
PCNSFVKTBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Computes the residual of advective term using finite volume method.");
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid userobject");
  MooseEnum eqn("mass momentum energy scalar");
  params.addRequiredParam<MooseEnum>("eqn", eqn, "The equation you're solving.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this kernel applies to.");
  params.addParam<bool>("velocity_function_includes_rho",
                        false,
                        "Whether the provided superficial velocity function actually includes "
                        "multiplication by rho (e.g. the function is representative of momentum.");
  params.addParam<FunctionName>(
      NS::superficial_velocity + "_function",
      "An optional name of a vector function for the velocity. If not provided then the "
      "superficial velocity will be treated implicitly (e.g. we will use the interior value");
  params.addParam<FunctionName>(
      NS::pressure + "_function",
      "An optional name of a function for the pressure. If not provided then the pressure will be "
      "treated implicitly (e.g. we will use the interior value");
  params.addParam<FunctionName>(
      NS::T_fluid + "_function",
      "An optional name of a function for the fluid temperature. If not provided then the fluid "
      "temperature will be treated implicitly (e.g. we will use the interior value");
  params.addParam<MooseEnum>(
      "limiter", moose_limiter_type, "The limiter to apply during interpolation.");
  return params;
}

PCNSFVKTBC::PCNSFVKTBC(const InputParameters & params)
  : FVFluxBC(params),
    _fluid(getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _dim(_mesh.dimension()),
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
    _sup_vel_provided(isParamValid(NS::superficial_velocity)),
    _pressure_provided(isParamValid(NS::pressure)),
    _T_fluid_provided(isParamValid(NS::T_fluid)),
    _sup_vel_function(_sup_vel_provided ? &getFunction(NS::superficial_velocity + "_function")
                                        : nullptr),
    _pressure_function(_pressure_provided ? &getFunction(NS::pressure + "_function") : nullptr),
    _T_fluid_function(_T_fluid_provided ? &getFunction(NS::T_fluid + "_function") : nullptr),
    _velocity_function_includes_rho(getParam<bool>("velocity_function_includes_rho")),
    _limiter(Limiter::build(LimiterType(int(getParam<MooseEnum>("limiter")))))
{
  if ((_eqn == "momentum") && !isParamValid("momentum_component"))
    paramError("eqn",
               "If 'momentum' is specified for 'eqn', then you must provide a parameter "
               "value for 'momentum_component'");
}

ADReal
PCNSFVKTBC::computeQpResidual()
{
  mooseAssert(_eps_elem[_qp] == _eps_neighbor[_qp], "the porosities need to be the same");
  const auto eps_interior = _eps_elem[_qp];
  const auto eps_boundary = eps_interior;

  const auto ft = _face_info->faceType(_var.name());
  const bool out_of_elem = (ft == FaceInfo::VarFaceNeighbors::ELEM);
  const auto normal = out_of_elem ? _face_info->normal() : Point(-_face_info->normal());
  const auto & sup_vel_x_interior_center =
      out_of_elem ? _sup_vel_x_elem[_qp] : _sup_vel_x_neighbor[_qp];
  const auto & sup_vel_y_interior_center =
      out_of_elem ? _sup_vel_y_elem[_qp] : _sup_vel_y_neighbor[_qp];
  const auto & sup_vel_z_interior_center =
      out_of_elem ? _sup_vel_z_elem[_qp] : _sup_vel_z_neighbor[_qp];
  const auto & pressure_interior_center =
      out_of_elem ? _pressure_elem[_qp] : _pressure_neighbor[_qp];
  const auto & T_fluid_interior_center = out_of_elem ? _T_fluid_elem[_qp] : _T_fluid_neighbor[_qp];

  const auto & grad_sup_vel_x_interior_center =
      out_of_elem ? _grad_sup_vel_x_elem[_qp] : _grad_sup_vel_x_neighbor[_qp];
  const auto & grad_sup_vel_y_interior_center =
      out_of_elem ? _grad_sup_vel_y_elem[_qp] : _grad_sup_vel_y_neighbor[_qp];
  const auto & grad_sup_vel_z_interior_center =
      out_of_elem ? _grad_sup_vel_z_elem[_qp] : _grad_sup_vel_z_neighbor[_qp];
  const auto & grad_pressure_interior_center =
      out_of_elem ? _grad_pressure_elem[_qp] : _grad_pressure_neighbor[_qp];
  const auto & grad_T_fluid_interior_center =
      out_of_elem ? _grad_T_fluid_elem[_qp] : _grad_T_fluid_neighbor[_qp];
  const auto & interior_centroid =
      out_of_elem ? _face_info->elemCentroid() : _face_info->neighborCentroid();
  const auto dCf = _face_info->faceCentroid() - interior_centroid;

  ADReal pressure_boundary, pressure_interior;
  if (_pressure_provided)
  {
    pressure_boundary = _pressure_function->value(_t, _face_info->faceCentroid());
    pressure_interior = interpolate(*_limiter,
                                    pressure_interior_center,
                                    pressure_boundary,
                                    grad_pressure_interior_center,
                                    *_face_info,
                                    out_of_elem);
  }
  else
    pressure_boundary = pressure_interior =
        pressure_interior_center + grad_pressure_interior_center * dCf;

  ADReal T_fluid_boundary, T_fluid_interior;
  if (_T_fluid_provided)
  {
    T_fluid_boundary = _T_fluid_function->value(_t, _face_info->faceCentroid());
    T_fluid_interior = interpolate(*_limiter,
                                   T_fluid_interior_center,
                                   T_fluid_boundary,
                                   grad_T_fluid_interior_center,
                                   *_face_info,
                                   out_of_elem);
  }
  else
    T_fluid_boundary = T_fluid_interior =
        T_fluid_interior_center + grad_T_fluid_interior_center * dCf;

  // Need rho_boundary potentially for sup_vel calculations
  const auto rho_boundary = _fluid.rho_from_p_T(pressure_boundary, T_fluid_boundary);

  ADReal sup_vel_x_boundary, sup_vel_x_interior;
  if (_sup_vel_provided)
  {
    sup_vel_x_boundary = _sup_vel_function->vectorValue(_t, _face_info->faceCentroid())(0);
    if (_velocity_function_includes_rho)
      sup_vel_x_boundary /= rho_boundary;
    sup_vel_x_interior = interpolate(*_limiter,
                                     sup_vel_x_interior_center,
                                     sup_vel_x_boundary,
                                     grad_sup_vel_x_interior_center,
                                     *_face_info,
                                     out_of_elem);
  }
  else
    sup_vel_x_boundary = sup_vel_x_interior =
        sup_vel_x_interior_center + grad_sup_vel_x_interior_center * dCf;

  ADReal sup_vel_y_boundary = 0, sup_vel_y_interior = 0;
  if (_dim >= 2)
  {
    if (_sup_vel_provided)
    {
      sup_vel_y_boundary = _sup_vel_function->vectorValue(_t, _face_info->faceCentroid())(1);
      if (_velocity_function_includes_rho)
        sup_vel_y_boundary /= rho_boundary;
      sup_vel_y_interior = interpolate(*_limiter,
                                       sup_vel_y_interior_center,
                                       sup_vel_y_boundary,
                                       grad_sup_vel_y_interior_center,
                                       *_face_info,
                                       out_of_elem);
    }
    else
      sup_vel_y_boundary = sup_vel_y_interior =
          sup_vel_y_interior_center + grad_sup_vel_y_interior_center * dCf;
  }

  ADReal sup_vel_z_boundary = 0, sup_vel_z_interior = 0;
  if (_dim >= 3)
  {
    if (_sup_vel_provided)
    {
      sup_vel_z_boundary = _sup_vel_function->vectorValue(_t, _face_info->faceCentroid())(2);
      if (_velocity_function_includes_rho)
        sup_vel_z_boundary /= rho_boundary;
      sup_vel_z_interior = interpolate(*_limiter,
                                       sup_vel_z_interior_center,
                                       sup_vel_z_boundary,
                                       grad_sup_vel_z_interior_center,
                                       *_face_info,
                                       out_of_elem);
    }
    else
      sup_vel_z_boundary = sup_vel_z_interior =
          sup_vel_z_interior_center + grad_sup_vel_z_interior_center * dCf;
  }

  const auto sup_vel_interior =
      VectorValue<ADReal>(sup_vel_x_interior, sup_vel_y_interior, sup_vel_z_interior);
  const auto u_interior = sup_vel_interior / eps_interior;
  const auto rho_interior = _fluid.rho_from_p_T(pressure_interior, T_fluid_interior);
  const auto specific_volume_interior = 1. / rho_interior;
  const auto e_interior = _fluid.e_from_T_v(T_fluid_interior, specific_volume_interior);
  const auto sup_vel_boundary =
      VectorValue<ADReal>(sup_vel_x_boundary, sup_vel_y_boundary, sup_vel_z_boundary);
  const auto u_boundary = sup_vel_boundary / eps_boundary;
  const auto specific_volume_boundary = 1. / rho_boundary;
  const auto e_boundary = _fluid.e_from_T_v(T_fluid_boundary, specific_volume_boundary);

  const auto c_interior = _fluid.c_from_v_e(specific_volume_interior, e_interior);
  const auto c_boundary = _fluid.c_from_v_e(specific_volume_boundary, e_boundary);

  const auto sup_vel_interior_normal = sup_vel_interior * normal;
  const auto sup_vel_boundary_normal = sup_vel_boundary * normal;
  const auto u_interior_normal = u_interior * normal;
  const auto u_boundary_normal = u_boundary * normal;

  const auto a_interior =
      std::max(std::abs(u_interior_normal + c_interior), std::abs(u_interior_normal - c_interior));
  const auto a_boundary =
      std::max(std::abs(u_boundary_normal + c_boundary), std::abs(u_boundary_normal - c_boundary));
  // Second term is to avoid new nonzero mallocs
  const auto a = std::max(a_interior, a_boundary) + 0 * a_interior + 0 * a_boundary;

  if (_eqn == "mass")
    return 0.5 * (sup_vel_interior_normal * rho_interior + sup_vel_boundary_normal * rho_boundary -
                  a * (rho_boundary - rho_interior));
  else if (_eqn == "momentum")
  {
    const auto rhou_interior = u_interior(_index) * rho_interior;
    const auto rhou_boundary = u_boundary(_index) * rho_boundary;
    return 0.5 *
           (sup_vel_interior_normal * rhou_interior + sup_vel_boundary_normal * rhou_boundary +
            (eps_interior * pressure_interior + eps_boundary * pressure_boundary) * normal(_index) -
            a * (rhou_boundary - rhou_interior));
  }
  else if (_eqn == "energy")
  {
    const auto ht_interior =
        e_interior + 0.5 * u_interior * u_interior + pressure_interior / rho_interior;
    const auto ht_boundary =
        e_boundary + 0.5 * u_boundary * u_boundary + pressure_boundary / rho_boundary;
    const auto rho_ht_interior = rho_interior * ht_interior;
    const auto rho_ht_boundary = rho_boundary * ht_boundary;
    return 0.5 *
           (sup_vel_interior_normal * rho_ht_interior + sup_vel_boundary_normal * rho_ht_boundary -
            a * (rho_ht_boundary - rho_ht_interior));
  }
  else
    mooseError("Unrecognized enum type ", _eqn);
}
