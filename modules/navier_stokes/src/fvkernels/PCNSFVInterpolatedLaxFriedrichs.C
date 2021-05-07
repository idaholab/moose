//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVInterpolatedLaxFriedrichs.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Limiter.h"

using namespace Moose::FV;

registerMooseObject("MooseApp", PCNSFVInterpolatedLaxFriedrichs);

InputParameters
PCNSFVInterpolatedLaxFriedrichs::validParams()
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

PCNSFVInterpolatedLaxFriedrichs::PCNSFVInterpolatedLaxFriedrichs(const InputParameters & params)
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

void
PCNSFVInterpolatedLaxFriedrichs::computeResidual(const FaceInfo & fi)
{
  if (onBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  const auto r = MetaPhysicL::raw_value(computeQpResidual());

  const auto ft = fi.faceType(_var.name());
  if (ft != FaceInfo::VarFaceNeighbors::BOTH)
    mooseError("This object should only be run on internal faces. If on a boundary face, then "
               "PCNSFVInterpolatedLaxFriedrichsBC should be used instead");

  // residual contribution of this kernel to the elem element
  prepareVectorTag(_assembly, _var.number());
  _local_re(0) = r;
  accumulateTaggedLocalResidual();

  // residual contribution of this kernel to the neighbor element
  prepareVectorTagNeighbor(_assembly, _var.number());
  _local_re(0) = -r;
  accumulateTaggedLocalResidual();
}

void
PCNSFVInterpolatedLaxFriedrichs::computeJacobian(const FaceInfo & fi)
{
  if (onBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  const auto r = computeQpResidual();

  const auto ft = fi.faceType(_var.name());
  if (ft != FaceInfo::VarFaceNeighbors::BOTH)
    mooseError("This object should only be run on internal faces. If on a boundary face, then "
               "PCNSFVInterpolatedLaxFriedrichsBC should be used instead");

  mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");
  _assembly.processDerivatives(r, _var.dofIndices()[0], _matrix_tags);

  mooseAssert(_var.dofIndicesNeighbor().size() == 1,
              "We're currently built to use CONSTANT MONOMIALS");
  _assembly.processDerivatives(-r, _var.dofIndicesNeighbor()[0], _matrix_tags);
}

ADReal
PCNSFVInterpolatedLaxFriedrichs::computeQpResidual()
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

  ADReal c_elem, c_neighbor;
  Real dc_elem_dv, dc_elem_de, dc_neighbor_dv, dc_neighbor_de;
  _fluid.c_from_v_e(
      specific_volume_elem.value(), e_elem.value(), c_elem.value(), dc_elem_dv, dc_elem_de);
  _fluid.c_from_v_e(specific_volume_neighbor.value(),
                    e_neighbor.value(),
                    c_neighbor.value(),
                    dc_neighbor_dv,
                    dc_neighbor_de);
  c_elem.derivatives() =
      dc_elem_dv * specific_volume_elem.derivatives() + dc_elem_de * e_elem.derivatives();
  c_neighbor.derivatives() = dc_neighbor_dv * specific_volume_neighbor.derivatives() +
                             dc_neighbor_de * e_neighbor.derivatives();

  const auto Sf = _face_info->faceArea() * _face_info->faceCoord() * _face_info->normal();
  auto vSf_elem = sup_vel_elem * Sf;
  auto vSf_neighbor = sup_vel_neighbor * Sf;
  const auto cSf_elem = c_elem * Sf.norm();
  const auto cSf_neighbor = c_neighbor * Sf.norm();
  // Create this to avoid new nonzero mallocs
  const ADReal dummy_psi = 0 * (vSf_elem + cSf_elem + vSf_neighbor + cSf_neighbor);
  auto psi_elem = std::max({vSf_elem + cSf_elem, vSf_neighbor + cSf_neighbor, ADReal(0)});
  psi_elem += dummy_psi;
  auto psi_neighbor = std::min({vSf_elem - cSf_elem, vSf_neighbor - cSf_neighbor, ADReal(0)});
  psi_neighbor += dummy_psi;
  const auto alpha_elem = psi_elem / (psi_elem - psi_neighbor);
  const auto alpha_neighbor = 1. - alpha_elem;
  const auto psi_max = std::max(std::abs(psi_elem), std::abs(psi_neighbor));
  const auto omega = psi_neighbor * alpha_elem;
  vSf_elem *= alpha_elem;
  vSf_neighbor *= alpha_neighbor;
  const auto adjusted_vSf_elem = vSf_elem - omega;
  const auto adjusted_vSf_neighbor = vSf_neighbor + omega;
  auto adjusted_vSf_max = std::max(std::abs(adjusted_vSf_elem), std::abs(adjusted_vSf_neighbor));
  adjusted_vSf_max += 0 * (adjusted_vSf_elem + adjusted_vSf_neighbor);

  if (_eqn == "mass")
    return adjusted_vSf_elem * rho_elem + adjusted_vSf_neighbor * rho_neighbor;
  else if (_eqn == "momentum")
  {
    const auto rhou_elem = u_elem(_index) * rho_elem;
    const auto rhou_neighbor = u_neighbor(_index) * rho_neighbor;
    return adjusted_vSf_elem * rhou_elem + adjusted_vSf_neighbor * rhou_neighbor +
           (alpha_elem * _eps_elem[_qp] * pressure_elem +
            alpha_neighbor * _eps_neighbor[_qp] * pressure_neighbor) *
               Sf(_index);
  }
  else if (_eqn == "energy")
  {
    const auto ht_elem = e_elem + 0.5 * u_elem * u_elem + pressure_elem / rho_elem;
    const auto ht_neighbor =
        e_neighbor + 0.5 * u_neighbor * u_neighbor + pressure_neighbor / rho_neighbor;
    const auto rho_ht_elem = rho_elem * ht_elem;
    const auto rho_ht_neighbor = rho_neighbor * ht_neighbor;
    return adjusted_vSf_elem * rho_ht_elem + adjusted_vSf_neighbor * rho_ht_neighbor +
           // This term removes artifacts at boundaries in the particle velocity solution. Note that
           // if instead of using pressure, one uses porosity*pressure then the solution is totally
           // wrong
           omega * (pressure_elem - pressure_neighbor);
  }
  else if (_eqn == "scalar")
    return adjusted_vSf_elem * rho_elem * _scalar_elem[_qp] +
           adjusted_vSf_neighbor * rho_neighbor * _scalar_neighbor[_qp];
  else
    mooseError("Unrecognized enum type ", _eqn);
}
