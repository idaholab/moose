//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVStrongBC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"
#include "MfrPostprocessor.h"

registerMooseObject("NavierStokesApp", PCNSFVStrongBC);

InputParameters
PCNSFVStrongBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Computes the residual of advective term using finite volume method.");
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid properties userobject");
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
      NS::superficial_velocity,
      "An optional name of a vector function for the velocity. If not provided then the "
      "superficial velocity will be treated implicitly (e.g. we will use the interior value");
  params.addParam<FunctionName>(
      NS::pressure,
      "An optional name of a function for the pressure. If not provided then the pressure will be "
      "treated implicitly (e.g. we will use the interior value");
  params.addParam<FunctionName>(
      NS::T_fluid,
      "An optional name of a function for the fluid temperature. If not provided then the fluid "
      "temperature will be treated implicitly (e.g. we will use the interior value");
  params.addParam<FunctionName>(
      "scalar",
      "A function describing the value of the scalar at the boundary. If this function is not "
      "provided, then the interior value will be used.");
  params.addParam<UserObjectName>("mfr_postprocessor",
                                  "A postprocessor used for outputting mass flow rates on the same "
                                  "boundary this object acts on");
  return params;
}

PCNSFVStrongBC::PCNSFVStrongBC(const InputParameters & params)
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
    _sup_vel_function(_sup_vel_provided ? &getFunction(NS::superficial_velocity) : nullptr),
    _pressure_function(_pressure_provided ? &getFunction(NS::pressure) : nullptr),
    _T_fluid_function(_T_fluid_provided ? &getFunction(NS::T_fluid) : nullptr),
    _scalar_elem(_u),
    _scalar_neighbor(_u_neighbor),
    _grad_scalar_elem((_eqn == "scalar") ? &_var.adGradSln() : nullptr),
    _grad_scalar_neighbor((_eqn == "scalar") ? &_var.adGradSlnNeighbor() : nullptr),
    _scalar_function_provided(isParamValid("scalar")),
    _scalar_function(_scalar_function_provided ? &getFunction("scalar") : nullptr),
    _velocity_function_includes_rho(getParam<bool>("velocity_function_includes_rho")),
    _mfr_pp(
        isParamValid("mfr_postprocessor")
            ? &const_cast<MfrPostprocessor &>(getUserObject<MfrPostprocessor>("mfr_postprocessor"))
            : nullptr)
{
  if ((_eqn == "momentum") && !isParamValid("momentum_component"))
    paramError("eqn",
               "If 'momentum' is specified for 'eqn', then you must provide a parameter "
               "value for 'momentum_component'");
  if ((_eqn != "momentum") && isParamValid("momentum_component"))
    paramError("momentum_component",
               "'momentum_component' should not be specified when the 'eqn' is not 'momentum'");
}

ADReal
PCNSFVStrongBC::computeQpResidual()
{
  const auto ft = _face_info->faceType(_var.name());
  const bool out_of_elem = (ft == FaceInfo::VarFaceNeighbors::ELEM);
  const auto normal = out_of_elem ? _face_info->normal() : Point(-_face_info->normal());

  const auto & sup_vel_x_interior = out_of_elem ? _sup_vel_x_elem[_qp] : _sup_vel_x_neighbor[_qp];
  const auto & sup_vel_y_interior = out_of_elem ? _sup_vel_y_elem[_qp] : _sup_vel_y_neighbor[_qp];
  const auto & sup_vel_z_interior = out_of_elem ? _sup_vel_z_elem[_qp] : _sup_vel_z_neighbor[_qp];
  const auto & pressure_interior = out_of_elem ? _pressure_elem[_qp] : _pressure_neighbor[_qp];
  const auto & T_fluid_interior = out_of_elem ? _T_fluid_elem[_qp] : _T_fluid_neighbor[_qp];

  const auto & grad_sup_vel_x_interior =
      out_of_elem ? _grad_sup_vel_x_elem[_qp] : _grad_sup_vel_x_neighbor[_qp];
  const auto & grad_sup_vel_y_interior =
      out_of_elem ? _grad_sup_vel_y_elem[_qp] : _grad_sup_vel_y_neighbor[_qp];
  const auto & grad_sup_vel_z_interior =
      out_of_elem ? _grad_sup_vel_z_elem[_qp] : _grad_sup_vel_z_neighbor[_qp];
  const auto & grad_pressure_interior =
      out_of_elem ? _grad_pressure_elem[_qp] : _grad_pressure_neighbor[_qp];
  const auto & grad_T_fluid_interior =
      out_of_elem ? _grad_T_fluid_elem[_qp] : _grad_T_fluid_neighbor[_qp];
  const auto & interior_centroid =
      out_of_elem ? _face_info->elemCentroid() : _face_info->neighborCentroid();
  const auto dCf = _face_info->faceCentroid() - interior_centroid;
  const auto eps_interior = out_of_elem ? _eps_elem[_qp] : _eps_neighbor[_qp];

  const auto pressure_boundary =
      _pressure_provided ? ADReal(_pressure_function->value(_t, _face_info->faceCentroid()))
                         : pressure_interior + grad_pressure_interior * dCf;
  const auto T_fluid_boundary =
      _T_fluid_provided ? ADReal(_T_fluid_function->value(_t, _face_info->faceCentroid()))
                        : T_fluid_interior + grad_T_fluid_interior * dCf;
  const auto rho_boundary = _fluid.rho_from_p_T(pressure_boundary, T_fluid_boundary);

  ADReal sup_vel_x_boundary;
  if (_sup_vel_provided)
  {
    sup_vel_x_boundary = _sup_vel_function->vectorValue(_t, _face_info->faceCentroid())(0);
    if (_velocity_function_includes_rho)
      sup_vel_x_boundary /= rho_boundary;
  }
  else
    sup_vel_x_boundary = sup_vel_x_interior + grad_sup_vel_x_interior * dCf;

  ADReal sup_vel_y_boundary = 0;
  if (_dim >= 2)
  {
    if (_sup_vel_provided)
    {
      sup_vel_y_boundary = _sup_vel_function->vectorValue(_t, _face_info->faceCentroid())(1);
      if (_velocity_function_includes_rho)
        sup_vel_y_boundary /= rho_boundary;
    }
    else
      sup_vel_y_boundary = sup_vel_y_interior + grad_sup_vel_y_interior * dCf;
  }

  ADReal sup_vel_z_boundary = 0;
  if (_dim >= 3)
  {
    if (_sup_vel_provided)
    {
      sup_vel_z_boundary = _sup_vel_function->vectorValue(_t, _face_info->faceCentroid())(2);
      if (_velocity_function_includes_rho)
        sup_vel_z_boundary /= rho_boundary;
    }
    else
      sup_vel_z_boundary = sup_vel_z_interior + grad_sup_vel_z_interior * dCf;
  }

  const VectorValue<ADReal> sup_vel_boundary(
      sup_vel_x_boundary, sup_vel_y_boundary, sup_vel_z_boundary);
  const auto eps_boundary = eps_interior;
  const auto u_boundary = sup_vel_boundary / eps_boundary;
  const auto e_boundary = _fluid.e_from_p_T(pressure_boundary, T_fluid_boundary);

  if (_eqn == "mass")
  {
    const ADReal mfr = rho_boundary * sup_vel_boundary * normal;
    if (_mfr_pp)
      _mfr_pp->setMfr(_face_info, mfr.value(), false);
    return mfr;
  }
  else if (_eqn == "momentum")
  {
    const auto rhou_boundary = u_boundary(_index) * rho_boundary;
    return rhou_boundary * sup_vel_boundary * normal +
           eps_boundary * pressure_boundary * normal(_index);
  }
  else if (_eqn == "energy")
  {
    const auto ht_boundary =
        e_boundary + 0.5 * u_boundary * u_boundary + pressure_boundary / rho_boundary;
    const auto rho_ht_boundary = rho_boundary * ht_boundary;
    return rho_ht_boundary * sup_vel_boundary * normal;
  }
  else if (_eqn == "scalar")
  {
    const auto & scalar_interior = out_of_elem ? _scalar_elem[_qp] : _scalar_neighbor[_qp];
    const auto & grad_scalar_interior =
        out_of_elem ? (*_grad_scalar_elem)[_qp] : (*_grad_scalar_neighbor)[_qp];
    const auto scalar_boundary =
        _scalar_function_provided ? ADReal(_scalar_function->value(_t, _face_info->faceCentroid()))
                                  : scalar_interior + grad_scalar_interior * dCf;
    return rho_boundary * scalar_boundary * sup_vel_boundary * normal;
  }
  else
    mooseError("Unrecognized equation type ", _eqn);
}
