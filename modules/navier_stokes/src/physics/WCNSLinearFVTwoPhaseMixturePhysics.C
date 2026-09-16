//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVTwoPhaseMixturePhysics.h"
#include "WCNSFVTwoPhaseMixturePhysics.h"
#include "WCNSFVFluidHeatTransferPhysics.h"
#include "WCNSLinearFVFluidHeatTransferPhysics.h"
#include "WCNSFVFlowPhysics.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp", WCNSLinearFVTwoPhaseMixturePhysics);
registerWCNSFVScalarTransportBaseTasks("NavierStokesApp", WCNSLinearFVTwoPhaseMixturePhysics);
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVTwoPhaseMixturePhysics,
                    "add_interpolation_method_physics");
registerMooseAction("NavierStokesApp", WCNSLinearFVTwoPhaseMixturePhysics, "add_material");
registerMooseAction("NavierStokesApp", WCNSLinearFVTwoPhaseMixturePhysics, "check_integrity");

InputParameters
WCNSLinearFVTwoPhaseMixturePhysics::validParams()
{
  // The parameters are mostly the same being the linear and nonlinear version
  InputParameters params = WCNSLinearFVScalarTransportPhysics::validParams();
  WCNSFVTwoPhaseMixturePhysics::renamePassiveScalarToMixtureParams(params);
  // The flow physics is obtained from the scalar transport base class
  // The fluid heat transfer physics is retrieved even if unspecified
  params.addParam<PhysicsName>(
      "fluid_heat_transfer_physics",
      "NavierStokesFV",
      "WCNSLinearFVFluidHeatTransferPhysics generating the fluid energy equation");
  params += WCNSFVTwoPhaseMixturePhysics::commonMixtureParams();
  params.addParamNamesToGroup("fluid_heat_transfer_physics", "Phase change");
  params.addClassDescription("Define the additional terms for a mixture model for the two phase "
                             "weakly-compressible Navier Stokes equations using the linearized "
                             "segregated finite volume discretization");

  MooseEnum drag_model("schiller-naumann distorted-particle automatic ishii-zuber",
                       "schiller-naumann");
  params.addParam<MooseEnum>(
      "slip_drag_model",
      drag_model,
      "Drag law closing the slip velocity when 'use_dispersed_phase_drag_model' is set. See "
      "LinearWCNSFV2PSlipVelocityFunctorMaterial. The distorted particle law reproduces Ishii's "
      "drift velocity correlation and is the appropriate one for bubbles large enough to deform.");
  params.addParam<MooseFunctorName>(
      "surface_tension",
      "Surface tension between the phases, required by the distorted particle drag law.");
  params.addParam<Real>(
      "slip_swarm_exponent",
      0.0,
      "Exponent of the hindrance factor (1 - alpha)^p on the slip velocity, correcting the "
      "single-particle drag laws for the presence of a swarm. 0.75 reproduces the swarm "
      "dependence of Ishii's drift velocity correlation.");
  params.addParam<MooseFunctorName>(
      "slip_friction_pressure_gradient",
      "0",
      "Frictional pressure gradient of the two phase flow, used by the 'ishii-zuber' drag law to "
      "carry the effect of wall friction on the relative velocity.");
  params.addParam<MooseFunctorName>(
      "slip_single_particle_friction_pressure_gradient",
      "0",
      "Frictional pressure gradient of the corresponding single particle system, used by the "
      "'ishii-zuber' drag law in both its terminal velocity and its concentration factor.");
  params.addParamNamesToGroup("slip_drag_model surface_tension slip_swarm_exponent "
                              "slip_friction_pressure_gradient "
                              "slip_single_particle_friction_pressure_gradient",
                              "Friction model");

  params.addParam<InterpolationMethodName>(
      "phase_drift_advection_interpolation",
      "Scheme for the drift flux in the phase transport equation. The drift is always interpolated "
      "separately from the mixture flux; this chooses which scheme the drift half uses, and "
      "defaults to 'phase_advection_interpolation'. Setting it is what allows a limiter to act on "
      "the drift correction alone.");
  params.addParamNamesToGroup("phase_drift_advection_interpolation", "Numerical scheme");

  // This is added to match a nonlinear test result. If the underlying issue is fixed, remove it
  params.addParam<bool>("add_gravity_term_in_slip_velocity",
                        true,
                        "Whether to add the gravity term in the slip velocity vector computation");
  return params;
}

WCNSLinearFVTwoPhaseMixturePhysics::WCNSLinearFVTwoPhaseMixturePhysics(
    const InputParameters & parameters)
  : WCNSLinearFVScalarTransportPhysics(parameters),
    _add_phase_equation(_has_scalar_equation),
    _phase_1_fraction_name(getParam<MooseFunctorName>("phase_1_fraction_name")),
    _phase_2_fraction_name(_passive_scalar_names[0]),
    _phase_1_density(getParam<MooseFunctorName>("phase_1_density_name")),
    _phase_1_viscosity(getParam<MooseFunctorName>("phase_1_viscosity_name")),
    _phase_1_specific_heat(getParam<MooseFunctorName>("phase_1_specific_heat_name")),
    _phase_1_thermal_conductivity(getParam<MooseFunctorName>("phase_1_thermal_conductivity_name")),
    _phase_2_density(getParam<MooseFunctorName>("phase_2_density_name")),
    _phase_2_viscosity(getParam<MooseFunctorName>("phase_2_viscosity_name")),
    _phase_2_specific_heat(getParam<MooseFunctorName>("phase_2_specific_heat_name")),
    _phase_2_thermal_conductivity(getParam<MooseFunctorName>("phase_2_thermal_conductivity_name")),
    _use_external_mixture_properties(getParam<bool>("use_external_mixture_properties")),
    _use_drift_flux(getParam<bool>("add_drift_flux_momentum_terms")),
    _use_advection_slip(getParam<bool>("add_advection_slip_term"))
{
  // Check that only one scalar was passed, as we are using vector parameters
  if (_passive_scalar_names.size() > 1)
    paramError("phase_fraction_name", "Only one phase fraction currently supported.");
  if (_passive_scalar_inlet_functors.size() > 1)
    paramError("phase_fraction_inlet_functors", "Only one phase fraction currently supported");

  // Retrieve the fluid energy equation if it exists
  if (isParamValid("fluid_heat_transfer_physics"))
  {
    _fluid_energy_physics = getCoupledPhysics<WCNSLinearFVFluidHeatTransferPhysics>(
        getParam<PhysicsName>("fluid_heat_transfer_physics"), true);
    // Check for a missing parameter / do not support isolated physics for now
    if (!_fluid_energy_physics &&
        !getCoupledPhysics<const WCNSLinearFVFluidHeatTransferPhysics>(true).empty())
      paramError(
          "fluid_heat_transfer_physics",
          "We currently do not support creating both a phase transport equation and fluid heat "
          "transfer physics that are not coupled together");
    if (_fluid_energy_physics && _fluid_energy_physics->hasEnergyEquation())
      _has_energy_equation = true;
    else
      _has_energy_equation = false;
  }
  else
  {
    _has_energy_equation = false;
    _fluid_energy_physics = nullptr;
  }

  // Check that the mixture parameters are correctly in use in the other physics
  if (_has_energy_equation)
  {
    if (_fluid_energy_physics->densityName() != "rho_mixture")
      mooseError(
          "Density name for Physics '", _fluid_energy_physics->name(), "' should be 'rho_mixture'");
    if (_fluid_energy_physics->getSpecificHeatName() != "cp_mixture")
      mooseError("Specific heat name for Physics '",
                 _fluid_energy_physics->name(),
                 "' should be 'cp_mixture'");
  }
  if (_flow_equations_physics)
    if (_flow_equations_physics->densityName() != "rho_mixture")
      mooseError("Density name for Physics ,",
                 _flow_equations_physics->name(),
                 "' should be 'rho_mixture'");

  if (_verbose)
  {
    if (_flow_equations_physics)
      mooseInfoRepeated("Coupled to fluid flow physics " + _flow_equations_physics->name());
    if (_has_energy_equation)
      mooseInfoRepeated("Coupled to fluid heat transfer physics " + _fluid_energy_physics->name());
  }

  // Parameter checking
  // The two models are not consistent
  if (isParamSetByUser("alpha_exchange") && getParam<bool>("add_phase_change_energy_term"))
    paramError("alpha_exchange",
               "A phase exchange coefficient cannot be specified if the phase change is handled "
               "with a phase change heat loss model");
  if (_phase_1_fraction_name == _phase_2_fraction_name)
    paramError("phase_1_fraction_name",
               "First phase fraction name should be different from second phase fraction name");
  if (_use_drift_flux && _use_advection_slip)
    paramError("add_drift_flux_momentum_terms",
               "Drift flux model cannot be used at the same time as the advection slip model");
  if (!getParam<bool>("add_drift_flux_momentum_terms"))
    errorDependentParameter("add_drift_flux_momentum_terms", "true", {"density_interp_method"});
  if (!getParam<bool>("use_dispersed_phase_drag_model"))
    errorDependentParameter("use_dispersed_phase_drag_model", "true", {"particle_diameter"});
  if (getParam<bool>("use_dispersed_phase_drag_model") &&
      isParamSetByUser("slip_linear_friction_name"))
    paramError("slip_linear_friction_name",
               "A prescribed slip friction factor cannot be combined with "
               "'use_dispersed_phase_drag_model'. The drag model forms its particle Reynolds "
               "number from the slip velocity, so it is solved inside the slip closure rather "
               "than supplied to it.");
}

void
WCNSLinearFVTwoPhaseMixturePhysics::checkIntegrity() const
{
  if (!_flow_equations_physics)
    mooseError("Expected a flow physics");

  // Check the mesh for unsupported sknewness + buoyancy
  if (_flow_equations_physics->gravityVector().norm() > 0)
  {
    const auto tol = 1e-2;
    if (_problem->mesh().allFaceInfo().empty())
      _problem->mesh().setupFiniteVolumeMeshData();
    for (const auto & fi : _problem->mesh().allFaceInfo())
    {
      if (fi.skewnessCorrectionVector().norm() > tol * fi.dCNMag())
        mooseError("Face with centroid ",
                   fi.faceCentroid(),
                   " requires skewness correction. We currently do not support mixture flow with "
                   "buoyancy and mesh skewness. Please contact a MOOSE or Navier Stokes module "
                   "developer if you require this.");
    }
  }
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addFVKernels()
{
  WCNSLinearFVScalarTransportPhysics::addFVKernels();

  // The phase fraction equation is an advection equation whose boundedness rests on the time
  // derivative. Solved steady, the discrete operator loses the property that each cell value is a
  // convex combination of its neighbours wherever the dispersed phase velocity is compressive, and
  // the phase fraction can leave [0, 1].
  if (_add_phase_equation && !isTransient())
    mooseInfoRepeated(
        "The phase transport equation is being solved without a time derivative. Its boundedness "
        "is not guaranteed in that form and the phase fraction may leave [0, 1], which in turn "
        "drives the mixture properties and the slip closure outside their range of validity. "
        "Whether it bites depends on how large the drift flux is: it is benign when the slip is "
        "small, as it is without gravity, and it is not when the slip is a significant part of "
        "the transport. Prefer a transient executioner, marching to steady state if a steady "
        "answer is wanted.");

  if (_add_phase_equation && isParamSetByUser("alpha_exchange"))
    addPhaseInterfaceTerm();

  if (_fluid_energy_physics && _fluid_energy_physics->hasEnergyEquation() &&
      getParam<bool>("add_phase_change_energy_term"))
    addPhaseChangeEnergySource();

  if (_flow_equations_physics && _flow_equations_physics->hasFlowEquations() && _use_drift_flux)
    addPhaseDriftFluxTerm();
  if (_flow_equations_physics && _flow_equations_physics->hasFlowEquations() && _use_advection_slip)
    addAdvectionSlipTerm();
}

void
WCNSLinearFVTwoPhaseMixturePhysics::setSlipVelocityParams(InputParameters & params) const
{
  // Only the phase advection kernel interpolates the drift separately; the momentum and energy
  // drift terms are kernels of their own already.
  if (isParamValid("phase_drift_advection_interpolation") &&
      params.have_parameter<InterpolationMethodName>("slip_advected_interp_method_name"))
    params.set<InterpolationMethodName>("slip_advected_interp_method_name") =
        getParam<InterpolationMethodName>("phase_drift_advection_interpolation");

  params.set<MooseFunctorName>("u_slip") = "vel_drift_x";
  if (dimension() >= 2)
    params.set<MooseFunctorName>("v_slip") = "vel_drift_y";
  if (dimension() >= 3)
    params.set<MooseFunctorName>("w_slip") = "vel_drift_z";
}

void
WCNSLinearFVTwoPhaseMixturePhysics::setRelativeVelocityParams(InputParameters & params) const
{
  params.set<MooseFunctorName>("u_slip") = "vel_slip_x";
  if (dimension() >= 2)
    params.set<MooseFunctorName>("v_slip") = "vel_slip_y";
  if (dimension() >= 3)
    params.set<MooseFunctorName>("w_slip") = "vel_slip_z";
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addPhaseInterfaceTerm()
{
  // Recreate the phase interface term from existing kernels
  {
    auto params = getFactory().getValidParams("LinearFVReaction");
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _phase_2_fraction_name;
    params.set<MooseFunctorName>("coeff") = getParam<MooseFunctorName>(NS::alpha_exchange);
    getProblem().addLinearFVKernel(
        "LinearFVReaction", prefix() + "phase_interface_reaction", params);
  }
  {
    auto params = getFactory().getValidParams("LinearFVSource");
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _phase_2_fraction_name;
    params.set<MooseFunctorName>("source_density") = _phase_1_fraction_name;
    params.set<MooseFunctorName>("scaling_factor") = getParam<MooseFunctorName>(NS::alpha_exchange);
    getProblem().addLinearFVKernel("LinearFVSource", prefix() + "phase_interface_source", params);
  }
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addPhaseChangeEnergySource()
{
  mooseError("Phase change energy source not implemented at this time for linear finite volume");
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addPhaseDriftFluxTerm()
{
  const std::vector<std::string> components = {"x", "y", "z"};
  for (const auto dim : make_range(dimension()))
  {
    const auto object_type = "LinearWCNSFV2PMomentumDriftFlux";
    auto params = getFactory().getValidParams(object_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _flow_equations_physics->getVelocityNames()[dim];
    setRelativeVelocityParams(params);
    params.set<MooseFunctorName>("rho_d") = _phase_2_density;
    params.set<MooseFunctorName>("rho_c") = _phase_1_density;
    params.set<MooseFunctorName>("fraction_dispersed") = _phase_2_fraction_name;
    params.set<MooseEnum>("momentum_component") = components[dim];
    params.set<MooseEnum>("density_interp_method") = getParam<MooseEnum>("density_interp_method");
    params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
    getProblem().addLinearFVKernel(object_type, prefix() + "drift_flux_" + components[dim], params);
  }
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addAdvectionSlipTerm()
{
  mooseError("Phase advection slip not implemented at this time for linear finite volume");
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addMaterials()
{
  // Add the phase fraction variable, for output purposes mostly
  if (!getProblem().hasFunctor(_phase_1_fraction_name, /*thread_id=*/0))
  {
    auto params = getFactory().getValidParams("ParsedFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<std::string>("expression") = "1 - " + _phase_2_fraction_name;
    params.set<std::vector<std::string>>("functor_names") = {_phase_2_fraction_name};
    params.set<std::string>("property_name") = _phase_1_fraction_name;
    params.set<std::vector<std::string>>("output_properties") = {_phase_1_fraction_name};
    params.set<std::vector<OutputName>>("outputs") = {"all"};
    getProblem().addMaterial("ParsedFunctorMaterial", prefix() + "phase_1_fraction", params);

    // One of the phase fraction should exist though (either as a variable or set by a
    // NSLiquidFractionAux)
    if (!getProblem().hasFunctor(_phase_2_fraction_name, /*thread_id=*/0))
      paramError("Phase 2 fraction should be defined as a variable or auxiliary variable");
  }
  if (!getProblem().hasFunctor(_phase_2_fraction_name, /*thread_id=*/0))
  {
    auto params = getFactory().getValidParams("ParsedFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<std::string>("expression") = "1 - " + _phase_1_fraction_name;
    params.set<std::vector<std::string>>("functor_names") = {_phase_1_fraction_name};
    params.set<std::string>("property_name") = _phase_2_fraction_name;
    params.set<std::vector<std::string>>("output_properties") = {_phase_2_fraction_name};
    params.set<std::vector<OutputName>>("outputs") = {"all"};
    getProblem().addMaterial("ParsedFunctorMaterial", prefix() + "phase_2_fraction", params);
  }

  // Compute mixture properties
  if (!_use_external_mixture_properties)
  {
    auto params = getFactory().getValidParams("WCNSLinearFVMixtureFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<std::vector<MooseFunctorName>>("prop_names") = {
        "rho_mixture", "mu_mixture", "cp_mixture", "k_mixture"};
    // The phase_1 and phase_2 assignments are only local to this object.
    // We use the phase 2 variable to save a functor evaluation as we expect
    // the phase 2 variable to be a nonlinear variable in the phase transport equation
    params.set<std::vector<MooseFunctorName>>("phase_2_names") = {_phase_1_density,
                                                                  _phase_1_viscosity,
                                                                  _phase_1_specific_heat,
                                                                  _phase_1_thermal_conductivity};
    params.set<std::vector<MooseFunctorName>>("phase_1_names") = {_phase_2_density,
                                                                  _phase_2_viscosity,
                                                                  _phase_2_specific_heat,
                                                                  _phase_2_thermal_conductivity};
    params.set<MooseFunctorName>("phase_1_fraction") = _phase_2_fraction_name;
    if (getParam<bool>("output_all_properties"))
      params.set<std::vector<OutputName>>("outputs") = {"all"};
    params.set<bool>("limit_phase_fraction") = true;
    getProblem().addMaterial(
        "WCNSLinearFVMixtureFunctorMaterial", prefix() + "mixture_material", params);
  }

  // Compute slip terms as functors, used by the drift flux kernels. The drag model needs them too,
  // since its particle Reynolds number is formed from the slip velocity.
  if (_use_advection_slip || _use_drift_flux || _add_phase_equation ||
      getParam<bool>("use_dispersed_phase_drag_model"))
  {
    mooseAssert(_flow_equations_physics, "We must have coupled to this");
    const std::vector<std::string> vel_components = {"u", "v", "w"};
    const std::vector<std::string> components = {"x", "y", "z"};
    for (const auto dim : make_range(dimension()))
    {
      const auto object_type = "LinearWCNSFV2PSlipVelocityFunctorMaterial";
      auto params = getFactory().getValidParams(object_type);
      assignBlocks(params, _blocks);
      params.set<MooseFunctorName>("slip_velocity_name") = "vel_slip_" + components[dim];
      params.set<MooseEnum>("momentum_component") = components[dim];
      for (const auto j : make_range(dimension()))
        params.set<SolverVariableName>(vel_components[j]) =
            _flow_equations_physics->getVelocityNames()[j];
      // The buoyancy factor of the closure is (rho_d - rho_m) / rho_d, which carries the mixture
      // density, not the continuous phase density, see VTT Publications 288 equation (58)
      params.set<MooseFunctorName>(NS::density) = "rho_mixture";
      // The slip closure forms both the relaxation time and the particle Reynolds number
      // from the continuous phase viscosity, not from the mixture viscosity
      params.set<MooseFunctorName>(NS::mu) = _phase_1_viscosity;
      params.set<MooseFunctorName>("rho_d") = _phase_2_density;
      params.set<MooseFunctorName>("fraction_dispersed") = _phase_2_fraction_name;
      // The phase equation is advected with the diffusion velocity, which this object derives
      // from the slip velocity it computes
      params.set<MooseFunctorName>("drift_velocity_name") = "vel_drift_" + components[dim];
      params.set<MooseFunctorName>("volumetric_drift_velocity_name") =
          "vel_volumetric_drift_" + components[dim];
      if (getParam<bool>("add_gravity_term_in_slip_velocity"))
        params.set<RealVectorValue>("gravity") = _flow_equations_physics->gravityVector();
      // The drag model is solved inside the slip closure rather than read from the drag material.
      // The particle Reynolds number of the correlation is formed from the slip velocity, so a
      // drag functor built the correct way depends on the slip velocity and cannot also be an
      // input to it; solving the correlation and the force balance together breaks that loop.
      if (getParam<bool>("use_dispersed_phase_drag_model"))
      {
        params.set<bool>("use_dispersed_phase_drag_model") = true;
        params.set<MooseFunctorName>("rho_c") = _phase_1_density;
        params.set<MooseEnum>("drag_model") = getParam<MooseEnum>("slip_drag_model");
        params.set<Real>("swarm_exponent") = getParam<Real>("slip_swarm_exponent");
        params.set<MooseFunctorName>("friction_pressure_gradient") =
            getParam<MooseFunctorName>("slip_friction_pressure_gradient");
        params.set<MooseFunctorName>("single_particle_friction_pressure_gradient") =
            getParam<MooseFunctorName>("slip_single_particle_friction_pressure_gradient");
        if (isParamValid("surface_tension"))
          params.set<MooseFunctorName>("surface_tension") =
              getParam<MooseFunctorName>("surface_tension");
      }
      else if (isParamValid("slip_linear_friction_name"))
        params.set<MooseFunctorName>("linear_coef_name") =
            getParam<MooseFunctorName>("slip_linear_friction_name");
      else if (_flow_equations_physics)
      {
        if (!_flow_equations_physics->getLinearFrictionCoefName().empty())
          params.set<MooseFunctorName>("linear_coef_name") =
              _flow_equations_physics->getLinearFrictionCoefName();
        else
          params.set<MooseFunctorName>("linear_coef_name") = "0";
      }
      else
        paramError("slip_linear_friction_name",
                   "LinearWCNSFV2PSlipVelocityFunctorMaterial created by this Physics required a "
                   "scalar field linear friction factor.");
      params.set<MooseFunctorName>("particle_diameter") =
          getParam<MooseFunctorName>("particle_diameter");
      if (getParam<bool>("output_all_properties"))
      {
        if (!isTransient())
          params.set<std::vector<OutputName>>("outputs") = {"all"};
        else
          paramInfo("output_all_properties",
                    "Slip velocity functor material output currently unsupported in Physics "
                    "in transient conditions.");
      }
      getProblem().addMaterial(object_type, prefix() + "slip_" + components[dim], params);
    }
  }

  // Add a default drag model for a dispersed phase
  if (getParam<bool>("use_dispersed_phase_drag_model"))
  {
    const std::vector<std::string> vel_components = {"u", "v", "w"};
    const std::vector<std::string> components = {"x", "y", "z"};

    const auto drag_type = "NSFVDispersePhaseDragFunctorMaterial";
    auto params = getFactory().getValidParams(drag_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>("drag_coef_name") = "Darcy_coefficient";
    // The particle Reynolds number is formed from the slip velocity and the continuous phase
    // properties, which is its definition. The material evaluates in AD; the linear finite
    // volume friction kernel takes the raw value through the functor wrapper.
    for (const auto j : make_range(dimension()))
      params.set<MooseFunctorName>(vel_components[j]) = "vel_slip_" + components[j];
    params.set<MooseFunctorName>(NS::density) = _phase_1_density;
    params.set<MooseFunctorName>(NS::mu) = _phase_1_viscosity;
    params.set<MooseFunctorName>("particle_diameter") =
        getParam<MooseFunctorName>("particle_diameter");
    if (getParam<bool>("output_all_properties"))
      params.set<std::vector<OutputName>>("outputs") = {"all"};
    getProblem().addMaterial(drag_type, prefix() + "dispersed_drag", params);
  }
}
