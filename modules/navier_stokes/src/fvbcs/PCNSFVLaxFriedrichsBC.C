//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVLaxFriedrichsBC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"

registerMooseObject("MooseApp", PCNSFVLaxFriedrichsBC);

InputParameters
PCNSFVLaxFriedrichsBC::validParams()
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
  params.addParam<bool>(
      "implicit_state_var",
      false,
      "Whether the fourth variable in the system, e.g. density or pressure, should be treated "
      "implicitly, e.g. use the interior value, or calculated explicitly using the other three "
      "variables boundary information (recognizing that at least one of these other three "
      "variables *must* be implicit (for subsonic flows)) and the equation of state. I will note "
      "that treating the state variable implicitly doesn't even appear to allow a converged "
      "solution in initial testing, so I'll probably remove this option soon.");
  params.addParam<MaterialPropertyName>(
      "scalar_prop_name",
      "An optional material property name that can be used to specify an advected material "
      "property. If this is not supplied the variable variable will be used.");
  params.addParam<FunctionName>(
      "scalar",
      "A function describing the value of the scalar at the boundary. If this function is not "
      "provided, then the interior value will be used.");
  return params;
}

PCNSFVLaxFriedrichsBC::PCNSFVLaxFriedrichsBC(const InputParameters & params)
  : FVFluxBC(params),
    _fluid(getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _superficial_vel_elem(getADMaterialProperty<RealVectorValue>(NS::superficial_velocity)),
    _superficial_vel_neighbor(
        getNeighborADMaterialProperty<RealVectorValue>(NS::superficial_velocity)),
    _rho_elem(getADMaterialProperty<Real>(NS::density)),
    _rho_neighbor(getNeighborADMaterialProperty<Real>(NS::density)),
    _T_fluid_elem(getADMaterialProperty<Real>(NS::T_fluid)),
    _T_fluid_neighbor(getNeighborADMaterialProperty<Real>(NS::T_fluid)),
    _pressure_elem(getADMaterialProperty<Real>(NS::pressure)),
    _pressure_neighbor(getNeighborADMaterialProperty<Real>(NS::pressure)),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity)),
    _eqn(getParam<MooseEnum>("eqn")),
    _index(getParam<MooseEnum>("momentum_component")),
    _svel_provided(isParamValid(NS::superficial_velocity)),
    _pressure_provided(isParamValid(NS::pressure)),
    _T_fluid_provided(isParamValid(NS::T_fluid)),
    _svel_function(_svel_provided ? &getFunction(NS::superficial_velocity) : nullptr),
    _pressure_function(_pressure_provided ? &getFunction(NS::pressure) : nullptr),
    _T_fluid_function(_T_fluid_provided ? &getFunction(NS::T_fluid) : nullptr),
    _implicit_state_var(getParam<bool>("implicit_state_var")),
    _scalar_elem(isParamValid("scalar_prop_name")
                     ? getADMaterialProperty<Real>("scalar_prop_name").get()
                     : _u),
    _scalar_neighbor(isParamValid("scalar_prop_name")
                         ? getNeighborADMaterialProperty<Real>("scalar_prop_name").get()
                         : _u_neighbor),
    _scalar_function_provided(isParamValid("scalar")),
    _scalar_function(_scalar_function_provided ? &getFunction("scalar") : nullptr),
    _velocity_function_includes_rho(getParam<bool>("velocity_function_includes_rho"))
{
  if ((_eqn == "momentum") && !isParamValid("momentum_component"))
    paramError("eqn",
               "If 'momentum' is specified for 'eqn', then you must provide a parameter "
               "value for 'momentum_component'");
}

void
PCNSFVLaxFriedrichsBC::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto ft = fi.faceType(_var.name());

  const auto r = MetaPhysicL::raw_value(computeQpResidual());

  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
    prepareVectorTag(_assembly, _var.number());
  else if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    prepareVectorTagNeighbor(_assembly, _var.number());
  else if (ft == FaceInfo::VarFaceNeighbors::BOTH)
    mooseError("A PCNSFVLaxFriedrichsBC is being triggered on an internal face with centroid: ",
               fi.faceCentroid());
  else
    mooseError(
        "A PCNSFVLaxFriedrichsBC is being triggered on a face which does not connect to a block ",
        "with the relevant finite volume variable. Its centroid: ",
        fi.faceCentroid());

  _local_re(0) = r;
  accumulateTaggedLocalResidual();
}

void
PCNSFVLaxFriedrichsBC::computeJacobian(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto ft = fi.faceType(_var.name());

  const auto r = computeQpResidual();

  const auto & dof_indices =
      (ft == FaceInfo::VarFaceNeighbors::ELEM) ? _var.dofIndices() : _var.dofIndicesNeighbor();

  mooseAssert(dof_indices.size() == 1, "We're currently built to use CONSTANT MONOMIALS");

  _assembly.processDerivatives(r, dof_indices[0], _matrix_tags);
}

ADReal
PCNSFVLaxFriedrichsBC::computeQpResidual()
{
  const auto ft = _face_info->faceType(_var.name());
  const bool out_of_elem = (ft == FaceInfo::VarFaceNeighbors::ELEM);
  const auto normal = out_of_elem ? _face_info->normal() : Point(-_face_info->normal());

  const auto & superficial_vel_interior =
      out_of_elem ? _superficial_vel_elem[_qp] : _superficial_vel_neighbor[_qp];
  const auto & pressure_interior = out_of_elem ? _pressure_elem[_qp] : _pressure_neighbor[_qp];
  const auto & T_fluid_interior = out_of_elem ? _T_fluid_elem[_qp] : _T_fluid_neighbor[_qp];
  const auto & rho_interior = out_of_elem ? _rho_elem[_qp] : _rho_neighbor[_qp];
  const auto eps_interior = out_of_elem ? _eps_elem[_qp] : _eps_neighbor[_qp];
  const auto u_interior = superficial_vel_interior / eps_interior;
  const auto e_interior = _fluid.e_from_p_rho(pressure_interior, rho_interior);
  const auto specific_volume_interior = 1. / rho_interior;

  const auto pressure_boundary =
      _pressure_provided ? ADReal(_pressure_function->value(_t, _face_info->faceCentroid()))
                         : pressure_interior;
  const auto T_fluid_boundary =
      _T_fluid_provided ? ADReal(_T_fluid_function->value(_t, _face_info->faceCentroid()))
                        : T_fluid_interior;
  const auto rho_boundary =
      _implicit_state_var ? rho_interior : _fluid.rho_from_p_T(pressure_boundary, T_fluid_boundary);
  const auto superficial_vel_boundary =
      _svel_provided
          ? ADRealVectorValue(_svel_function->vectorValue(_t, _face_info->faceCentroid())) /
                (_velocity_function_includes_rho ? rho_boundary : ADReal(1.))
          : static_cast<const TypeVector<ADReal> &>(superficial_vel_interior);
  const auto eps_boundary = eps_interior;
  const auto u_boundary = superficial_vel_boundary / eps_boundary;
  const auto e_boundary = _fluid.e_from_p_rho(pressure_boundary, rho_boundary);
  const auto specific_volume_boundary = 1. / rho_boundary;

  const auto Sf = _face_info->faceArea() * _face_info->faceCoord() * normal;
  auto vSf_interior = superficial_vel_interior * Sf;
  auto vSf_boundary = superficial_vel_boundary * Sf;

  ADReal c_interior, c_boundary;
  Real dc_interior_dv, dc_interior_de, dc_boundary_dv, dc_boundary_de;
  _fluid.c_from_v_e(specific_volume_interior.value(),
                    e_interior.value(),
                    c_interior.value(),
                    dc_interior_dv,
                    dc_interior_de);
  _fluid.c_from_v_e(specific_volume_boundary.value(),
                    e_boundary.value(),
                    c_boundary.value(),
                    dc_boundary_dv,
                    dc_boundary_de);
  c_interior.derivatives() = dc_interior_dv * specific_volume_interior.derivatives() +
                             dc_interior_de * e_interior.derivatives();
  c_boundary.derivatives() = dc_boundary_dv * specific_volume_boundary.derivatives() +
                             dc_boundary_de * e_boundary.derivatives();

  const auto cSf_interior = c_interior * Sf.norm();
  const auto cSf_boundary = c_boundary * Sf.norm();
  // Create this to avoid new nonzero mallocs
  const ADReal dummy_psi = 0 * (vSf_interior + cSf_interior + vSf_boundary + cSf_boundary);
  auto psi_interior =
      std::max({vSf_interior + cSf_interior, vSf_boundary + cSf_boundary, ADReal(0)});
  psi_interior += dummy_psi;
  auto psi_boundary =
      std::min({vSf_interior - cSf_interior, vSf_boundary - cSf_boundary, ADReal(0)});
  psi_boundary += dummy_psi;
  const auto alpha_interior = psi_interior / (psi_interior - psi_boundary);
  const auto alpha_boundary = 1. - alpha_interior;
  const auto psi_max = std::max(std::abs(psi_interior), std::abs(psi_boundary));
  const auto omega = psi_boundary * alpha_interior;
  vSf_interior *= alpha_interior;
  vSf_boundary *= alpha_boundary;
  const auto adjusted_vSf_interior = vSf_interior - omega;
  const auto adjusted_vSf_boundary = vSf_boundary + omega;
  auto adjusted_vSf_max =
      std::max(std::abs(adjusted_vSf_interior), std::abs(adjusted_vSf_boundary));
  adjusted_vSf_max += 0 * (adjusted_vSf_interior + adjusted_vSf_boundary);

  if (_eqn == "mass")
    return adjusted_vSf_interior * rho_interior + adjusted_vSf_boundary * rho_boundary;
  else if (_eqn == "momentum")
  {
    const auto rhou_interior = u_interior(_index) * rho_interior;
    const auto rhou_boundary = u_boundary(_index) * rho_boundary;
    return adjusted_vSf_interior * rhou_interior + adjusted_vSf_boundary * rhou_boundary +
           (alpha_interior * eps_interior * pressure_interior +
            alpha_boundary * eps_boundary * pressure_boundary) *
               Sf(_index);
  }
  else if (_eqn == "energy")
  {
    const auto ht_interior =
        e_interior + 0.5 * u_interior * u_interior + pressure_interior / rho_interior;
    const auto ht_boundary =
        e_boundary + 0.5 * u_boundary * u_boundary + pressure_boundary / rho_boundary;
    const auto rho_ht_interior = rho_interior * ht_interior;
    const auto rho_ht_boundary = rho_boundary * ht_boundary;
    return adjusted_vSf_interior * rho_ht_interior + adjusted_vSf_boundary * rho_ht_boundary +
           // This term removes artifacts at boundaries in the particle velocity solution. Note that
           // if instead of using pressure, one uses porosity*pressure then the solution is totally
           // wrong
           omega * (pressure_interior - pressure_boundary);
  }
  else if (_eqn == "scalar")
  {
    const auto & scalar_interior = out_of_elem ? _scalar_elem[_qp] : _scalar_neighbor[_qp];
    const auto scalar_boundary =
        _scalar_function_provided ? ADReal(_scalar_function->value(_t, _face_info->faceCentroid()))
                                  : scalar_interior;
    return adjusted_vSf_interior * rho_interior * scalar_interior +
           adjusted_vSf_boundary * rho_boundary * scalar_boundary;
  }
  else
    mooseError("Unrecognized enum type ", _eqn);
}
