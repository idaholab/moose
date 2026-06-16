//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVConservativeSharpInterfaceFlowPhysics.h"

#include "GeneralUserObject.h"
#include "Executioner.h"
#include "MooseUtils.h"
#include "NS.h"
#include "ConservativeSharpInterfaceRhieChowMassFlux.h"
#include "RhieChowMassFlux.h"
#include "TheWarehouse.h"

registerWCNSFVFlowPhysicsBaseTasks("NavierStokesApp",
                                   WCNSLinearFVConservativeSharpInterfaceFlowPhysics);
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeSharpInterfaceFlowPhysics,
                    "add_linear_fv_kernel");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeSharpInterfaceFlowPhysics,
                    "add_linear_fv_bc");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeSharpInterfaceFlowPhysics,
                    "add_functor_material");

InputParameters
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::validParams()
{
  InputParameters params = WCNSLinearFVFlowPhysics::validParams();

  params.addClassDescription("Define a linear-FV segregated sharp-interface flow solve using "
                             "velocity components as the primary momentum unknowns.");
  params.set<std::vector<std::string>>("velocity_variable") =
      std::vector<std::string>(NS::velocity_vector, NS::velocity_vector + 3);

  // Large-density-ratio sharp-interface work should default to a reduced / dynamic pressure solve.
  params.set<bool>("solve_for_dynamic_pressure") = true;
  params.transferParam<bool>(RhieChowMassFlux::validParams(),
                             "use_cached_momentum_predictor_operator");
  params.set<bool>("use_cached_momentum_predictor_operator") = true;

  params.transferParam<MooseFunctorName>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                                         "vof_rho_phi_functor");
  params.addParam<MooseFunctorName>(
      "volume_fraction_functor",
      "",
      "Volume-fraction functor used to generate sharp-interface geometry functors.");
  params.addParam<Real>(
      "near_interface_lower", 0.01, "Lower threshold used for the near-interface indicator.");
  params.addParam<Real>(
      "near_interface_upper", 0.99, "Upper threshold used for the near-interface indicator.");
  params.addParam<bool>(
      "create_geometry_functors",
      true,
      "Whether to automatically add the sharp-interface geometry functor material that produces "
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
  params.addParamNamesToGroup(
      "create_geometry_functors volume_fraction_functor geometry_delta_n "
      "clip_volume_fraction_for_geometry "
      "geometry_alpha_lower_bound geometry_alpha_upper_bound near_interface_lower "
      "near_interface_upper",
      "Sharp Interface Geometry");

  return params;
}

WCNSLinearFVConservativeSharpInterfaceFlowPhysics::
    WCNSLinearFVConservativeSharpInterfaceFlowPhysics(const InputParameters & parameters)
  : WCNSLinearFVFlowPhysics(parameters),
    _create_geometry_functors(getParam<bool>("create_geometry_functors"))
{
  if (!_solve_for_dynamic_pressure)
    paramError("solve_for_dynamic_pressure",
               "Conservative sharp-interface flow physics requires solve_for_dynamic_pressure = "
               "true.");
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::setMomentumTimeKernelParams(
    InputParameters & params) const
{
  params.set<bool>("use_old_state_factor_for_rhs") = true;
}

std::string
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::momentumOutletBCType(
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
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::setMomentumOutletBCParams(
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

  params.set<SolverVariableName>("u") = _velocity_names[0];
  if (dimension() >= 2)
    params.set<SolverVariableName>("v") = _velocity_names[1];
  if (dimension() >= 3)
    params.set<SolverVariableName>("w") = _velocity_names[2];
  params.set<MooseEnum>("momentum_component") = MooseEnum("x=0 y=1 z=2", NS::directions[component]);
  params.set<MooseFunctorName>("face_flux") = "corrected_face_phi";
}

std::string
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::pressureOutletBCType(
    const BoundaryName & /* boundary */, const MooseEnum & /* momentum_outlet_type */) const
{
  return "LinearFVPrghTotalPressureBC";
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::setPressureOutletBCParams(
    InputParameters & params,
    const BoundaryName & /* boundary */,
    const MooseEnum & /* momentum_outlet_type */) const
{
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  params.set<Point>("reference_pressure_point") = getParam<Point>("reference_pressure_point");
  params.set<SolverVariableName>("u") = _velocity_names[0];
  if (dimension() >= 2)
    params.set<SolverVariableName>("v") = _velocity_names[1];
  if (dimension() >= 3)
    params.set<SolverVariableName>("w") = _velocity_names[2];
  params.set<MooseFunctorName>("face_flux") = "corrected_face_phi";
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addWallPressureBC(
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
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::shouldAddWallPressureTwoTermExpansion() const
{
  if (getParam<bool>("pressure_two_term_bc_expansion"))
    paramWarning("pressure_two_term_bc_expansion",
                 "Ignoring pressure_two_term_bc_expansion on sharp-interface wall boundaries "
                 "because the reduced-pressure path now applies an explicit zero-normal-flux "
                 "pressure boundary condition.");

  return false;
}

MooseFunctorName
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::generatedGeometryFunctorName(
    const std::string & base_name) const
{
  return prefix() + base_name;
}

bool
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::shouldAddMomentumPressureKernels() const
{
  return false;
}

bool
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::shouldCreateGeometryFunctorMaterial() const
{
  return _create_geometry_functors &&
         !getParam<MooseFunctorName>("volume_fraction_functor").empty();
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addRhieChowUserObjects()
{
  mooseAssert(dimension(), "0-dimension not supported");

  std::vector<UserObject *> objs;
  getProblem()
      .theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);

  bool have_sharp_rc_uo = false;
  const auto this_block_ids =
      getSubdomainIDs(std::set<SubdomainName>(_blocks.begin(), _blocks.end()));
  for (const auto & obj : objs)
    if (dynamic_cast<RhieChowMassFlux *>(obj))
    {
      const auto rc_obj = dynamic_cast<RhieChowMassFlux *>(obj);
      const auto & rc_block_ids = rc_obj->blockIDs();
      const bool overlaps = rc_block_ids.empty() || this_block_ids.empty() ||
                            rc_block_ids.count(Moose::ANY_BLOCK_ID) ||
                            this_block_ids.count(Moose::ANY_BLOCK_ID) ||
                            MooseUtils::setsIntersect(rc_block_ids, this_block_ids);
      if (!overlaps)
        continue;

      if (dynamic_cast<ConservativeSharpInterfaceRhieChowMassFlux *>(obj))
        have_sharp_rc_uo = true;
      else
        mooseError("Sharp-interface flow physics '",
                   name(),
                   "' requires a ConservativeSharpInterfaceRhieChowMassFlux on blocks ",
                   Moose::stringify(_blocks),
                   ", but found existing RhieChowMassFlux '",
                   rc_obj->name(),
                   "' on overlapping blocks. Remove the stock Rhie-Chow object or use the "
                   "sharp-interface flow physics as the owner of the segregated flow coupling.");
    }

  if (have_sharp_rc_uo)
    return;

  const std::string u_names[3] = {"u", "v", "w"};
  const auto object_type = "ConservativeSharpInterfaceRhieChowMassFlux";
  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);

  for (unsigned int d = 0; d < dimension(); ++d)
    params.set<VariableName>(u_names[d]) = _velocity_names[d];

  params.set<VariableName>("pressure") = _pressure_name;
  params.set<std::string>("p_diffusion_kernel") = prefix() + "p_diffusion";
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseEnum>("pressure_projection_method") =
      getParam<MooseEnum>("pressure_projection_method");
  params.set<bool>("use_cached_momentum_predictor_operator") =
      getParam<bool>("use_cached_momentum_predictor_operator");
  params.set<bool>("split_momentum_predictor_operator") = true;
  if (parameters().isParamValid("gravity"))
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  if (_solve_for_dynamic_pressure)
    params.set<Point>("reference_pressure_point") = getParam<Point>("reference_pressure_point");
  params.set<MooseFunctorName>("vof_rho_phi_functor") =
      getParam<MooseFunctorName>("vof_rho_phi_functor");
  getProblem().addUserObject(object_type, rhieChowUOName(), params);
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addFunctorMaterials()
{
  WCNSLinearFVFlowPhysics::addFunctorMaterials();

  if (!shouldCreateGeometryFunctorMaterial())
  {
    if (_create_geometry_functors)
      paramWarning("volume_fraction_functor",
                   "No 'volume_fraction_functor' was supplied, so the sharp-interface geometry "
                   "functor material will not be created automatically.");
    return;
  }

  const std::string class_name = "ConservativeSharpInterfaceGeometryFunctorMaterial";
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
  getProblem().addFunctorMaterial(class_name, prefix() + "sharp_interface_geometry", params);
}

unsigned short
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  return 2;
}
