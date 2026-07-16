//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics.h"

#include "MooseUtils.h"
#include "NS.h"
#include "ConservativeDiffuseInterfaceRhieChowMassFlux.h"

registerWCNSFVFlowPhysicsBaseTasks("NavierStokesApp",
                                   WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics);
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics,
                    "add_linear_fv_kernel");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics,
                    "add_linear_fv_bc");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics,
                    "add_functor_material");

InputParameters
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::validParams()
{
  InputParameters params = WCNSLinearFVFlowPhysics::validParams();

  params.addClassDescription("Define a linear-FV segregated diffuse-interface flow solve using "
                             "velocity components as the primary momentum unknowns.");
  params.set<std::vector<std::string>>("velocity_variable") =
      std::vector<std::string>(NS::velocity_vector, NS::velocity_vector + 3);

  // Large-density-ratio diffuse-interface work should default to a reduced / dynamic pressure
  // solve.
  params.set<bool>("solve_for_dynamic_pressure") = true;
  params.transferParam<bool>(RhieChowMassFlux::validParams(),
                             "use_cached_momentum_predictor_operator");
  params.set<bool>("use_cached_momentum_predictor_operator") = true;

  params.transferParam<MooseFunctorName>(
      ConservativeDiffuseInterfaceRhieChowMassFlux::validParams(), "vof_rho_phi_functor");
  params.addParam<MooseFunctorName>(
      "volume_fraction_functor",
      "",
      "Volume-fraction functor used to generate diffuse-interface geometry functors.");
  params.addParam<Real>(
      "near_interface_lower", 0.01, "Lower threshold used for the near-interface indicator.");
  params.addParam<Real>(
      "near_interface_upper", 0.99, "Upper threshold used for the near-interface indicator.");
  params.addParam<bool>(
      "create_geometry_functors",
      true,
      "Whether to automatically add the diffuse-interface geometry functor material that produces "
      "face normals and capillary / hydrostatic face accelerations.");
  params.addParam<Real>(
      "geometry_delta_n",
      1e-8,
      "Regularization used in the geometry material when constructing interface unit normals.");
  params.addParam<bool>(
      "clip_volume_fraction_for_geometry",
      true,
      "Whether to clip the volume fraction in the geometry material before forming indicators.");
  params.addParam<Real>(
      "geometry_alpha_lower_bound", 0.0, "Lower clipping bound for the volume fraction.");
  params.addParam<Real>(
      "geometry_alpha_upper_bound", 1.0, "Upper clipping bound for the volume fraction.");
  params.addParam<MooseFunctorName>(
      "surface_tension_coefficient",
      "",
      "Surface-tension coefficient functor. Leave empty to disable capillary forcing.");
  params.addParamNamesToGroup(
      "create_geometry_functors volume_fraction_functor geometry_delta_n "
      "clip_volume_fraction_for_geometry "
      "geometry_alpha_lower_bound geometry_alpha_upper_bound near_interface_lower "
      "near_interface_upper surface_tension_coefficient",
      "Diffuse Interface Geometry");

  return params;
}

WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::
    WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics(const InputParameters & parameters)
  : WCNSLinearFVFlowPhysics(parameters),
    _create_geometry_functors(getParam<bool>("create_geometry_functors"))
{
  if (!_solve_for_dynamic_pressure)
    paramError("solve_for_dynamic_pressure",
               "Conservative diffuse-interface flow physics requires solve_for_dynamic_pressure = "
               "true.");
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::setMomentumTimeKernelParams(
    InputParameters & params) const
{
  params.set<bool>("use_old_state_factor_for_rhs") = true;
}

std::string
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::momentumOutletBCType(
    const BoundaryName & boundary, const MooseEnum & momentum_outlet_type) const
{
  const bool use_pressure_inlet_outlet_velocity =
      momentum_outlet_type == "fixed-pressure" ||
      momentum_outlet_type == "fixed-pressure-zero-gradient";

  return use_pressure_inlet_outlet_velocity
             ? "LinearFVPressureInletOutletMomentumBC"
             : WCNSLinearFVFlowPhysics::momentumOutletBCType(boundary, momentum_outlet_type);
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::setMomentumOutletBCParams(
    InputParameters & params,
    const BoundaryName & /* boundary */,
    const MooseEnum & momentum_outlet_type,
    const unsigned int component) const
{
  const bool use_pressure_inlet_outlet_velocity =
      momentum_outlet_type == "fixed-pressure" ||
      momentum_outlet_type == "fixed-pressure-zero-gradient";

  if (!use_pressure_inlet_outlet_velocity)
    return;

  setVelocitySolverVariableParams(params);
  params.set<MooseEnum>("momentum_component") = MooseEnum("x=0 y=1 z=2", NS::directions[component]);
  params.set<MooseFunctorName>("face_flux") = "corrected_face_phi";
}

std::string
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::pressureOutletBCType(
    const BoundaryName & /* boundary */, const MooseEnum & /* momentum_outlet_type */) const
{
  return "LinearFVPrghTotalPressureBC";
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::setPressureOutletBCParams(
    InputParameters & params,
    const BoundaryName & /* boundary */,
    const MooseEnum & /* momentum_outlet_type */) const
{
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  params.set<Point>("reference_pressure_point") = getParam<Point>("reference_pressure_point");
  setVelocitySolverVariableParams(params);
  params.set<MooseFunctorName>("face_flux") = "corrected_face_phi";
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::addWallPressureBC(
    const BoundaryName & boundary, const MooseEnum & momentum_wall_type)
{
  if (momentum_wall_type != "noslip")
    return;

  const std::string pressure_bc_type = "LinearFVPressureFluxBC";
  InputParameters pressure_params = getFactory().getValidParams(pressure_bc_type);
  pressure_params.set<std::vector<BoundaryName>>("boundary") = {boundary};
  pressure_params.set<LinearVariableName>("variable") = _pressure_name;
  pressure_params.set<MooseFunctorName>("constrained_pressure_normal_gradient") =
      "pressure_boundary_normal_gradient";
  pressure_params.set<MooseFunctorName>("HbyA_flux") = "phiHbyA";
  pressure_params.set<MooseFunctorName>("Ainv") = "pressure_Ainv";
  getProblem().addLinearFVBC(
      pressure_bc_type, _pressure_name + "_wall_flux_" + boundary, pressure_params);
}

bool
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::shouldAddWallPressureTwoTermExpansion() const
{
  if (getParam<bool>("pressure_two_term_bc_expansion"))
    paramWarning("pressure_two_term_bc_expansion",
                 "Ignoring pressure_two_term_bc_expansion on diffuse-interface wall boundaries "
                 "because the reduced-pressure path now applies an explicit zero-normal-flux "
                 "pressure boundary condition.");

  return false;
}

MooseFunctorName
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::generatedGeometryFunctorName(
    const std::string & base_name) const
{
  return prefix() + base_name;
}

bool
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::shouldAddMomentumPressureKernels() const
{
  return false;
}

bool
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::rhieChowUserObjectAppliesToBlocks(
    const RhieChowMassFlux & rc_obj) const
{
  const auto this_block_ids =
      getSubdomainIDs(std::set<SubdomainName>(_blocks.begin(), _blocks.end()));
  const auto & rc_block_ids = rc_obj.blockIDs();

  return rc_block_ids.empty() || this_block_ids.empty() ||
         rc_block_ids.count(Moose::ANY_BLOCK_ID) || this_block_ids.count(Moose::ANY_BLOCK_ID) ||
         MooseUtils::setsIntersect(rc_block_ids, this_block_ids);
}

bool
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::isCompatibleRhieChowUserObject(
    const UserObject & obj) const
{
  return dynamic_cast<const ConservativeDiffuseInterfaceRhieChowMassFlux *>(&obj);
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::checkIncompatibleRhieChowUserObject(
    const RhieChowMassFlux & rc_obj) const
{
  mooseError("Diffuse-interface flow physics '",
             name(),
             "' requires a ConservativeDiffuseInterfaceRhieChowMassFlux on blocks ",
             Moose::stringify(_blocks),
             ", but found existing RhieChowMassFlux '",
             rc_obj.name(),
             "' on overlapping blocks. Remove the stock Rhie-Chow object or use the "
             "diffuse-interface flow physics as the owner of the segregated flow coupling.");
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::setRhieChowUserObjectParams(
    InputParameters & params) const
{
  WCNSLinearFVFlowPhysics::setRhieChowUserObjectParams(params);
  params.set<bool>("use_cached_momentum_predictor_operator") =
      getParam<bool>("use_cached_momentum_predictor_operator");
  params.set<bool>("split_momentum_predictor_operator") = true;
  if (parameters().isParamValid("gravity"))
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  if (_solve_for_dynamic_pressure)
    params.set<Point>("reference_pressure_point") = getParam<Point>("reference_pressure_point");
  params.set<MooseFunctorName>("vof_rho_phi_functor") =
      getParam<MooseFunctorName>("vof_rho_phi_functor");

  const auto surface_tension_coefficient =
      getParam<MooseFunctorName>("surface_tension_coefficient");
  if (!surface_tension_coefficient.empty())
  {
    const auto volume_fraction_functor = getParam<MooseFunctorName>("volume_fraction_functor");
    if (volume_fraction_functor.empty())
      paramError("volume_fraction_functor",
                 "Surface tension requires the volume-fraction functor used by the least-squares "
                 "CSF curvature operator.");

    params.set<MooseFunctorName>("surface_tension_coefficient") = surface_tension_coefficient;
    params.set<MooseFunctorName>("surface_tension_volume_fraction_functor") =
        volume_fraction_functor;
  }
}

bool
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::shouldCreateGeometryFunctorMaterial() const
{
  return _create_geometry_functors &&
         !getParam<MooseFunctorName>("volume_fraction_functor").empty();
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::addFunctorMaterials()
{
  WCNSLinearFVFlowPhysics::addFunctorMaterials();

  if (!shouldCreateGeometryFunctorMaterial())
  {
    if (_create_geometry_functors)
      paramWarning("volume_fraction_functor",
                   "No 'volume_fraction_functor' was supplied, so the diffuse-interface geometry "
                   "functor material will not be created automatically.");
    return;
  }

  const std::string class_name = "ConservativeDiffuseInterfaceGeometryFunctorMaterial";
  auto params = getFactory().getValidParams(class_name);
  assignBlocks(params, _blocks);

  params.set<MooseFunctorName>("volume_fraction_functor") =
      getParam<MooseFunctorName>("volume_fraction_functor");

  params.set<bool>("clip_volume_fraction_for_geometry") =
      getParam<bool>("clip_volume_fraction_for_geometry");
  params.set<Real>("alpha_lower_bound") = getParam<Real>("geometry_alpha_lower_bound");
  params.set<Real>("alpha_upper_bound") = getParam<Real>("geometry_alpha_upper_bound");
  params.set<Real>("near_interface_lower") = getParam<Real>("near_interface_lower");
  params.set<Real>("near_interface_upper") = getParam<Real>("near_interface_upper");
  params.set<Real>("delta_n") = getParam<Real>("geometry_delta_n");

  params.set<MooseFunctorName>("delta_n_name") = generatedGeometryFunctorName("delta_n");
  params.set<MooseFunctorName>("near_interface_name") =
      generatedGeometryFunctorName("near_interface");
  params.set<MooseFunctorName>("alpha_gradient_name") =
      generatedGeometryFunctorName("alpha_gradient");
  params.set<MooseFunctorName>("interface_unit_normal_name") =
      generatedGeometryFunctorName("interface_unit_normal_face");
  getProblem().addFunctorMaterial(class_name, prefix() + "diffuse_interface_geometry", params);
}

unsigned short
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  return 2;
}
