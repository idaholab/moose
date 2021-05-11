//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVPrimitiveBC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"

registerMooseObject("MooseApp", PCNSFVPrimitiveBC);

InputParameters
PCNSFVPrimitiveBC::validParams()
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
  params.addRequiredCoupledVar(NS::pressure + "_var", "The pressure variable");
  params.addRequiredCoupledVar(NS::T_fluid + "_var", "The fluid energy");
  params.addRequiredCoupledVar(NS::superficial_velocity_x + "_var",
                               "The superficial velocity in the x direction");
  params.addCoupledVar(NS::superficial_velocity_y + "_var",
                       "The superficial velocity in the y direction");
  params.addCoupledVar(NS::superficial_velocity_z + "_var",
                       "The superficial velocity in the z direction");
  return params;
}

PCNSFVPrimitiveBC::PCNSFVPrimitiveBC(const InputParameters & params)
  : FVFluxBC(params),
    _fluid(getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _dim(_mesh.dimension()),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity)),
    _eqn(getParam<MooseEnum>("eqn")),
    _index(getParam<MooseEnum>("momentum_component")),
    _svel_provided(isParamValid(NS::superficial_velocity)),
    _pressure_provided(isParamValid(NS::pressure)),
    _T_fluid_provided(isParamValid(NS::T_fluid)),
    _svel_function(_svel_provided ? &getFunction(NS::superficial_velocity + "_function") : nullptr),
    _pressure_function(_pressure_provided ? &getFunction(NS::pressure + "_function") : nullptr),
    _T_fluid_function(_T_fluid_provided ? &getFunction(NS::T_fluid + "_function") : nullptr),
    _velocity_function_includes_rho(getParam<bool>("velocity_function_includes_rho")),
    _pressure_var(dynamic_cast<const MooseVariableFVReal *>(getFieldVar(NS::pressure + "_var", 0))),
    _T_fluid_var(dynamic_cast<const MooseVariableFVReal *>(getFieldVar(NS::T_fluid + "_var", 0))),
    _sup_vel_x_var(dynamic_cast<const MooseVariableFVReal *>(
        getFieldVar(NS::superficial_velocity_x + "_var", 0))),
    _sup_vel_y_var(isCoupled(NS::superficial_velocity_y + "_var")
                       ? dynamic_cast<const MooseVariableFVReal *>(
                             getFieldVar(NS::superficial_velocity_y + "_var", 0))
                       : nullptr),
    _sup_vel_z_var(isCoupled(NS::superficial_velocity_z + "_var")
                       ? dynamic_cast<const MooseVariableFVReal *>(
                             getFieldVar(NS::superficial_velocity_z + "_var", 0))
                       : nullptr)
{
  if ((_eqn == "momentum") && !isParamValid("momentum_component"))
    paramError("eqn",
               "If 'momentum' is specified for 'eqn', then you must provide a parameter "
               "value for 'momentum_component'");

  if (!_sup_vel_x_var)
    paramError(NS::superficial_velocity_x + "_var",
               NS::superficial_velocity_x + "_var",
               " must be a finite volume variable.");
  if (isCoupled(NS::superficial_velocity_y + "_var") && !_sup_vel_y_var)
    paramError(NS::superficial_velocity_y + "_var",
               NS::superficial_velocity_y + "_var",
               " must be a finite volume variable.");
  if (isCoupled(NS::superficial_velocity_z + "_var") && !_sup_vel_z_var)
    paramError(NS::superficial_velocity_z + "_var",
               NS::superficial_velocity_z + "_var",
               " must be a finite volume variable.");

  if (_dim >= 2 && !isCoupled(NS::superficial_velocity_y + "_var"))
    mooseError("For a mesh dimension of 2 or greater, the superficial velocity in the y direction "
               "must be provided via the '",
               NS::superficial_velocity_y + "_var",
               "' parameter");
  if (_dim >= 3 && !isCoupled(NS::superficial_velocity_z + "_var"))
    mooseError("For a mesh dimension of 2 or greater, the superficial velocity in the z direction "
               "must be provided via the '",
               NS::superficial_velocity_z + "_var",
               "' parameter");
}

ADReal
PCNSFVPrimitiveBC::computeQpResidual()
{
  mooseAssert(_eps_elem[_qp] == _eps_neighbor[_qp], "the porosities need to be the same");
  const auto eps_boundary = _eps_elem[_qp];
  const auto ft = _face_info->faceType(_var.name());
  const bool out_of_elem = (ft == FaceInfo::VarFaceNeighbors::ELEM);
  const auto normal = out_of_elem ? _face_info->normal() : Point(-_face_info->normal());

  const auto pressure_boundary = _pressure_var->getBoundaryFaceValue(*_face_info);
  const auto T_fluid_boundary = _T_fluid_var->getBoundaryFaceValue(*_face_info);
  const auto rho_boundary = _fluid.rho_from_p_T(pressure_boundary, T_fluid_boundary);
  const auto sup_vel_x_boundary = _sup_vel_x_var->getBoundaryFaceValue(*_face_info);
  const auto sup_vel_y_boundary =
      _dim >= 2 ? _sup_vel_y_var->getBoundaryFaceValue(*_face_info) : ADReal(0);
  const auto sup_vel_z_boundary =
      _dim >= 3 ? _sup_vel_z_var->getBoundaryFaceValue(*_face_info) : ADReal(0);

  const auto sup_vel_boundary =
      VectorValue<ADReal>(sup_vel_x_boundary, sup_vel_y_boundary, sup_vel_z_boundary);
  const auto u_boundary = sup_vel_boundary / eps_boundary;
  const auto normal_sup_vel_boundary = sup_vel_boundary * normal;

  if (_eqn == "mass")
    return normal_sup_vel_boundary * rho_boundary;
  else if (_eqn == "momentum")
    return normal_sup_vel_boundary * rho_boundary * u_boundary(_index) +
           eps_boundary * pressure_boundary * normal(_index);
  else if (_eqn == "energy")
  {
    const auto e_boundary = _fluid.e_from_p_rho(pressure_boundary, rho_boundary);
    const auto rho_ht_boundary =
        rho_boundary * (e_boundary + 0.5 * u_boundary * u_boundary) + pressure_boundary;
    return normal_sup_vel_boundary * rho_ht_boundary;
  }
  else
    mooseError("Unrecognized enum type ", _eqn);
}
