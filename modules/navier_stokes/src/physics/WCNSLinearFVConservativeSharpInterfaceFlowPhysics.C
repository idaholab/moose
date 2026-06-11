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
#include "MapConversionUtils.h"
#include "MooseUtils.h"
#include "NS.h"
#include "ConservativeSharpInterfaceRhieChowMassFlux.h"
#include "RhieChowMassFlux.h"
#include "TheWarehouse.h"

#include <algorithm>
#include <cctype>

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

namespace
{
bool
blocksOverlap(const std::set<SubdomainID> & lhs, const std::set<SubdomainID> & rhs)
{
  if (lhs.empty() || rhs.empty() || lhs.count(Moose::ANY_BLOCK_ID) ||
      rhs.count(Moose::ANY_BLOCK_ID))
    return true;

  auto lhs_it = lhs.begin();
  auto rhs_it = rhs.begin();
  while (lhs_it != lhs.end() && rhs_it != rhs.end())
  {
    if (*lhs_it < *rhs_it)
      ++lhs_it;
    else if (*rhs_it < *lhs_it)
      ++rhs_it;
    else
      return true;
  }

  return false;
}

std::string
sanitizeFunctorLabel(const std::string & input)
{
  std::string result = input;
  std::transform(result.begin(),
                 result.end(),
                 result.begin(),
                 [](unsigned char c) { return std::isalnum(c) ? static_cast<char>(c) : '_'; });
  return result;
}
}

InputParameters
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::validParams()
{
  InputParameters params = WCNSLinearFVFlowPhysicsBase::validParams();

  params.addClassDescription("Define a linear-FV segregated sharp-interface flow solve using "
                             "velocity components as the primary momentum unknowns.");
  params.set<std::vector<std::string>>("velocity_variable") =
      std::vector<std::string>(NS::velocity_vector, NS::velocity_vector + 3);

  // Large-density-ratio sharp-interface work should default to a reduced / dynamic pressure solve.
  params.set<bool>("solve_for_dynamic_pressure") = true;
  params.transferParam<bool>(RhieChowMassFlux::validParams(),
                             "use_cached_momentum_predictor_operator");
  params.set<bool>("use_cached_momentum_predictor_operator") = true;

  MooseEnum pressure_formulation("reduced total", "reduced");
  params.addParam<MooseEnum>(
      "pressure_formulation",
      pressure_formulation,
      "Whether the solved pressure variable is a reduced pressure or the total pressure.");

  params.addParam<bool>(
      "add_transient_projection_flux",
      true,
      "Whether to add an extra pressure-equation face-flux divergence for the transient "
      "projection.");
  params.addParam<bool>(
      "add_capillary_hydrostatic_flux",
      true,
      "Whether to add an extra pressure-equation face-flux divergence for capillary / hydrostatic "
      "predictor terms.");
  params.transferParam<bool>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                             "apply_pressure_velocity_writeback");
  params.transferParam<MooseFunctorName>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                                         "vof_rho_phi_functor");
  params.transferParam<MooseFunctorName>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                                         "volume_fraction_functor");
  params.transferParam<Real>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                             "near_interface_lower");
  params.transferParam<Real>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                             "near_interface_upper");
  params.transferParam<MooseFunctorName>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                                         "vof_alpha_phi_limited_functor");
  params.transferParam<MooseFunctorName>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                                         "liquid_density_functor");
  params.transferParam<MooseFunctorName>(ConservativeSharpInterfaceRhieChowMassFlux::validParams(),
                                         "gas_density_functor");
  params.addParam<MooseFunctorName>(
      "hydrostatic_momentum_source_x",
      "",
      "Optional x-component hydrostatic momentum-source density functor for the reduced-pressure "
      "momentum predictor. If left empty and the sharp-interface geometry functors are enabled, "
      "this physics object will auto-generate the functor name.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_momentum_source_y",
      "",
      "Optional y-component hydrostatic momentum-source density functor for the reduced-pressure "
      "momentum predictor. If left empty and the sharp-interface geometry functors are enabled, "
      "this physics object will auto-generate the functor name.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_momentum_source_z",
      "",
      "Optional z-component hydrostatic momentum-source density functor for the reduced-pressure "
      "momentum predictor. If left empty and the sharp-interface geometry functors are enabled, "
      "this physics object will auto-generate the functor name.");

  params.addParam<bool>(
      "create_geometry_functors",
      true,
      "Whether to automatically add the sharp-interface geometry functor material that produces "
      "face normals and capillary / hydrostatic face accelerations.");
  params.addParam<MooseFunctorName>(
      "volume_fraction_functor",
      "",
      "Volume-fraction / phase-fraction functor used by the geometry material. Leave empty to "
      "skip automatic creation of sharp-interface geometry objects.");
  params.addParam<MooseFunctorName>(
      "surface_tension_coefficient",
      "0",
      "Compatibility parameter for zero-surface-tension inputs. Nonzero capillary "
      "coupling is not part of this sharp-interface path.");

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
  params.addParam<Real>(
      "near_interface_lower", 0.01, "Lower threshold for the near-interface indicator.");
  params.addParam<Real>(
      "near_interface_upper", 0.99, "Upper threshold for the near-interface indicator.");
  params.addParamNamesToGroup("pressure_formulation add_transient_projection_flux "
                              "add_capillary_hydrostatic_flux apply_pressure_velocity_writeback",
                              "Sharp Interface Pressure Correction");

  params.addParamNamesToGroup("hydrostatic_momentum_source_x hydrostatic_momentum_source_y "
                              "hydrostatic_momentum_source_z",
                              "Sharp Interface Momentum Predictor");

  params.addParamNamesToGroup(
      "create_geometry_functors volume_fraction_functor surface_tension_coefficient "
      "geometry_delta_n clip_volume_fraction_for_geometry "
      "geometry_alpha_lower_bound geometry_alpha_upper_bound near_interface_lower "
      "near_interface_upper",
      "Sharp Interface Geometry");

  return params;
}

WCNSLinearFVConservativeSharpInterfaceFlowPhysics::
    WCNSLinearFVConservativeSharpInterfaceFlowPhysics(const InputParameters & parameters)
  : WCNSLinearFVFlowPhysicsBase(parameters),
    _pressure_formulation(getParam<MooseEnum>("pressure_formulation")),
    _add_transient_projection_flux(getParam<bool>("add_transient_projection_flux")),
    _add_capillary_hydrostatic_flux(getParam<bool>("add_capillary_hydrostatic_flux")),
    _create_geometry_functors(getParam<bool>("create_geometry_functors"))
{
  if (_pressure_formulation == "reduced" && !_solve_for_dynamic_pressure)
    paramError("solve_for_dynamic_pressure",
               "pressure_formulation = 'reduced' requires solve_for_dynamic_pressure = true.");

  if (_pressure_formulation == "total" && _solve_for_dynamic_pressure)
    paramError("solve_for_dynamic_pressure",
               "pressure_formulation = 'total' requires solve_for_dynamic_pressure = false.");
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addMomentumReducedPressureKernels()
{
  if (!_solve_for_dynamic_pressure || _pressure_formulation != "reduced" ||
      !_add_capillary_hydrostatic_flux)
    return;

  const std::string kernel_type = "LinearFVSource";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);

  const std::vector<std::string> comp_axis({"x", "y", "z"});
  const auto resolve_source_name =
      [this](const std::string & param_name, const std::string & generated_base_name)
  {
    const auto explicit_name = getParam<MooseFunctorName>(param_name);
    if (!explicit_name.empty())
      return explicit_name;

    return shouldCreateGeometryFunctorMaterial() ? generatedGeometryFunctorName(generated_base_name)
                                                 : MooseFunctorName("");
  };

  for (const auto d : make_range(dimension()))
  {
    params.set<LinearVariableName>("variable") = _velocity_names[d];

    const auto hydrostatic_source_name =
        resolve_source_name("hydrostatic_momentum_source_" + comp_axis[d],
                            "hydrostatic_momentum_source_" + comp_axis[d]);
    if (!hydrostatic_source_name.empty())
    {
      params.set<MooseFunctorName>("source_density") = hydrostatic_source_name;
      getProblem().addLinearFVKernel(
          kernel_type, prefix() + "ins_momentum_hydrostatic_source_" + comp_axis[d], params);
    }
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addOutletBC()
{
  const bool use_reduced_pressure_outlet_flux_bc =
      _solve_for_dynamic_pressure && _pressure_formulation == "reduced";
  unsigned int num_pressure_value_outlets = 0;
  for (const auto & [bdy, momentum_outlet_type] : _momentum_outlet_types)
    if (momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
      num_pressure_value_outlets++;

  if (num_pressure_value_outlets && num_pressure_value_outlets != _pressure_functors.size())
    paramError("pressure_functors",
               "Size (" + std::to_string(_pressure_functors.size()) +
                   ") is not the same as the number of pressure outlet boundaries in "
                   "'fixed-pressure/fixed-pressure-zero-gradient' (size " +
                   std::to_string(num_pressure_value_outlets) + ")");

  for (const auto & [outlet_bdy, momentum_outlet_type] : _momentum_outlet_types)
  {
    if (momentum_outlet_type == "zero-gradient" || momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      for (const auto d : make_range(dimension()))
      {
        const bool use_pressure_inlet_outlet_velocity =
            use_reduced_pressure_outlet_flux_bc &&
            (momentum_outlet_type == "fixed-pressure" ||
             momentum_outlet_type == "fixed-pressure-zero-gradient");

        const std::string bc_type = use_pressure_inlet_outlet_velocity
                                        ? "LinearFVPressureInletOutletMomentumBC"
                                        : "LinearFVAdvectionDiffusionOutflowBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
        params.set<bool>("use_two_term_expansion") =
            getParam<bool>("momentum_two_term_bc_expansion");
        params.set<LinearVariableName>("variable") = _velocity_names[d];

        if (use_pressure_inlet_outlet_velocity)
        {
          params.set<SolverVariableName>("u") = _velocity_names[0];
          if (dimension() >= 2)
            params.set<SolverVariableName>("v") = _velocity_names[1];
          if (dimension() >= 3)
            params.set<SolverVariableName>("w") = _velocity_names[2];
          params.set<MooseEnum>("momentum_component") = MooseEnum("x=0 y=1 z=2", NS::directions[d]);
          params.set<MooseFunctorName>("face_flux") = "corrected_face_phi";
        }

        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + outlet_bdy, params);
      }
    }

    if (momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
    {
      const std::string bc_type = use_reduced_pressure_outlet_flux_bc
                                      ? "LinearFVPrghTotalPressureBC"
                                      : "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<LinearVariableName>("variable") = _pressure_name;
      params.set<MooseFunctorName>("functor") = libmesh_map_find(_pressure_functors, outlet_bdy);
      params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
      if (use_reduced_pressure_outlet_flux_bc)
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
      getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + outlet_bdy, params);
    }
  }
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addWallsBC()
{
  const std::string u_names[3] = {"u", "v", "w"};
  for (const auto & [boundary_name, momentum_wall_type] : _momentum_wall_types)
  {
    if (momentum_wall_type == "noslip")
    {
      const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};

      for (const auto d : make_range(dimension()))
      {
        params.set<LinearVariableName>("variable") = _velocity_names[d];
        if (_momentum_wall_functors.count(boundary_name) == 0)
          params.set<MooseFunctorName>("functor") = "0";
        else
          params.set<MooseFunctorName>("functor") =
              generatedBoundaryMomentumFunctorName(boundary_name, d, "wall");

        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
      }

      const std::string pressure_bc_type = "LinearFVPressureFluxBC";
      InputParameters pressure_params = getFactory().getValidParams(pressure_bc_type);
      pressure_params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
      pressure_params.set<LinearVariableName>("variable") = _pressure_name;
      pressure_params.set<MooseFunctorName>("pressure_predictor_flux") = "pressure_predictor_flux";
      pressure_params.set<MooseFunctorName>("constrained_pressure_normal_gradient") =
          "pressure_boundary_normal_gradient";
      pressure_params.set<bool>("use_constrained_pressure_normal_gradient_only") = true;
      pressure_params.set<MooseFunctorName>("HbyA_flux") = "phiHbyA";
      pressure_params.set<MooseFunctorName>("Ainv") = "pressure_Ainv";
      getProblem().addLinearFVBC(
          pressure_bc_type, _pressure_name + "_wall_flux_" + boundary_name, pressure_params);
    }
    else if (momentum_wall_type == "symmetry")
    {
      {
        const std::string bc_type = "LinearFVVelocitySymmetryBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
        for (unsigned int d = 0; d < dimension(); ++d)
          params.set<SolverVariableName>(u_names[d]) = _velocity_names[d];

        for (const auto d : make_range(dimension()))
        {
          params.set<LinearVariableName>("variable") = _velocity_names[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
        }
      }
      {
        const std::string bc_type = "LinearFVPressureSymmetryBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
        params.set<LinearVariableName>("variable") = _pressure_name;
        params.set<MooseFunctorName>("HbyA_flux") = "phiHbyA";
        getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + boundary_name, params);
      }
    }
    else
      mooseError("Unsupported wall boundary condition type: " + std::string(momentum_wall_type));
  }

  if (getParam<bool>("pressure_two_term_bc_expansion"))
    paramWarning("pressure_two_term_bc_expansion",
                 "Ignoring pressure_two_term_bc_expansion on sharp-interface wall boundaries "
                 "because the reduced-pressure path now applies an explicit zero-normal-flux "
                 "pressure boundary condition.");
}

MooseFunctorName
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::generatedGeometryFunctorName(
    const std::string & base_name) const
{
  return prefix() + base_name;
}

MooseFunctorName
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::generatedBoundaryMomentumFunctorName(
    const BoundaryName & boundary, unsigned int component, const std::string & family) const
{
  return prefix() + family + "_momentum_" + sanitizeFunctorLabel(boundary) + "_" +
         NS::directions[component];
}

MooseFunctorName
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::inletVelocityFunctorName(
    const BoundaryName & boundary, const unsigned int component) const
{
  return generatedBoundaryMomentumFunctorName(boundary, component, "inlet");
}

MooseFunctorName
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::wallVelocityFunctorName(
    const BoundaryName & boundary, const unsigned int component) const
{
  if (_momentum_wall_functors.count(boundary) == 0)
    return "0";

  return generatedBoundaryMomentumFunctorName(boundary, component, "wall");
}

bool
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::shouldAddMomentumPressureKernels() const
{
  return !(_solve_for_dynamic_pressure && _pressure_formulation == "reduced" &&
           _add_capillary_hydrostatic_flux);
}

bool
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::shouldAddMomentumReducedPressureKernels() const
{
  return shouldAddMomentumPressureKernels();
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
      const bool overlaps = blocksOverlap(rc_obj->blockIDs(), this_block_ids);
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
  const bool use_face_based_reduced_pressure_predictor = _solve_for_dynamic_pressure &&
                                                         _pressure_formulation == "reduced" &&
                                                         _add_capillary_hydrostatic_flux;

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
  params.set<bool>("split_momentum_predictor_operator") = use_face_based_reduced_pressure_predictor;
  if (parameters().isParamValid("gravity"))
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  if (_solve_for_dynamic_pressure)
    params.set<Point>("reference_pressure_point") = getParam<Point>("reference_pressure_point");
  params.set<bool>("add_transient_projection_flux") = _add_transient_projection_flux;
  params.set<bool>("add_capillary_hydrostatic_flux") = _add_capillary_hydrostatic_flux;
  params.set<bool>("apply_pressure_velocity_writeback") =
      getParam<bool>("apply_pressure_velocity_writeback");
  params.set<MooseFunctorName>("vof_rho_phi_functor") =
      getParam<MooseFunctorName>("vof_rho_phi_functor");
  params.set<MooseFunctorName>("volume_fraction_functor") =
      getParam<MooseFunctorName>("volume_fraction_functor");
  params.set<Real>("near_interface_lower") = getParam<Real>("near_interface_lower");
  params.set<Real>("near_interface_upper") = getParam<Real>("near_interface_upper");
  params.set<MooseFunctorName>("vof_alpha_phi_limited_functor") =
      getParam<MooseFunctorName>("vof_alpha_phi_limited_functor");
  params.set<MooseFunctorName>("liquid_density_functor") =
      getParam<MooseFunctorName>("liquid_density_functor");
  params.set<MooseFunctorName>("gas_density_functor") =
      getParam<MooseFunctorName>("gas_density_functor");
  getProblem().addUserObject(object_type, rhieChowUOName(), params);
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addFunctorMaterials()
{
  if (parameters().isParamValid("gravity"))
  {
    const auto gravity_vector = getParam<RealVectorValue>("gravity");
    const std::vector<std::string> comp_axis({"x", "y", "z"});

    for (const auto d : make_range(dimension()))
      if (gravity_vector(d) != 0)
      {
        auto params = getFactory().getValidParams("ADParsedFunctorMaterial");
        assignBlocks(params, _blocks);
        params.set<bool>("enable_jit") = false;
        params.set<std::string>("expression") =
            _density_gravity_name + " * " + std::to_string(gravity_vector(d));
        if (!MooseUtils::parsesToReal(_density_gravity_name))
          params.set<std::vector<std::string>>("functor_names") = {_density_gravity_name};
        params.set<std::string>("property_name") = "rho_g_" + comp_axis[d];

        getProblem().addMaterial(
            "ADParsedFunctorMaterial", prefix() + "gravity_helper_" + comp_axis[d], params);
      }
  }

  addVelocityBoundaryInputFunctorMaterials();

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
  params.set<MooseFunctorName>("density_functor") = _density_name;

  if (parameters().isParamValid("gravity"))
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");

  if (_solve_for_dynamic_pressure)
    params.set<Point>("reference_pressure_point") = getParam<Point>("reference_pressure_point");

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
  params.set<MooseFunctorName>("hydrostatic_momentum_source_x_name") =
      generatedGeometryFunctorName("hydrostatic_momentum_source_x");
  params.set<MooseFunctorName>("hydrostatic_momentum_source_y_name") =
      generatedGeometryFunctorName("hydrostatic_momentum_source_y");
  params.set<MooseFunctorName>("hydrostatic_momentum_source_z_name") =
      generatedGeometryFunctorName("hydrostatic_momentum_source_z");

  getProblem().addFunctorMaterial(class_name, prefix() + "sharp_interface_geometry", params);
}

void
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addVelocityBoundaryInputFunctorMaterials()
{
  auto add_velocity_functor = [this](const MooseFunctorName & source_functor,
                                     const MooseFunctorName & target_functor,
                                     const std::string & object_suffix)
  {
    auto params = getFactory().getValidParams("ADParsedFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<bool>("enable_jit") = false;
    params.set<std::string>("expression") = source_functor;
    std::vector<std::string> functor_names;
    if (!MooseUtils::parsesToReal(source_functor))
      functor_names.push_back(source_functor);
    if (!functor_names.empty())
      params.set<std::vector<std::string>>("functor_names") = functor_names;
    params.set<std::string>("property_name") = target_functor;
    getProblem().addMaterial(
        "ADParsedFunctorMaterial", prefix() + object_suffix + "_" + target_functor, params);
  };

  for (const auto & [boundary, inlet_type] : _momentum_inlet_types)
    if (inlet_type == "fixed-velocity")
    {
      const auto & velocity_functors = libmesh_map_find(_momentum_inlet_functors, boundary);
      for (const auto d : make_range(dimension()))
        add_velocity_functor(velocity_functors[d],
                             generatedBoundaryMomentumFunctorName(boundary, d, "inlet"),
                             "inlet_velocity_bc");
    }

  for (const auto & [boundary, wall_type] : _momentum_wall_types)
    if (wall_type == "noslip" && _momentum_wall_functors.count(boundary))
    {
      const auto & velocity_functors = _momentum_wall_functors[boundary];
      for (const auto d : make_range(dimension()))
        add_velocity_functor(velocity_functors[d],
                             generatedBoundaryMomentumFunctorName(boundary, d, "wall"),
                             "wall_velocity_bc");
    }
}

unsigned short
WCNSLinearFVConservativeSharpInterfaceFlowPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  return 2;
}
