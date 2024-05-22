//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVTwoPhaseMixturePhysics.h"
#include "WCNSFVFluidHeatTransferPhysics.h"
#include "WCNSFVFlowPhysics.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp", WCNSFVTwoPhaseMixturePhysics);
registerWCNSFVScalarTransportBaseTasks("NavierStokesApp", WCNSFVTwoPhaseMixturePhysics);
registerMooseAction("NavierStokesApp", WCNSFVTwoPhaseMixturePhysics, "add_material");

InputParameters
WCNSFVTwoPhaseMixturePhysics::validParams()
{
  InputParameters params = WCNSFVScalarTransportPhysics::validParams();
  params.addClassDescription("Define the additional terms for a mixture model for the two phase "
                             "weakly-compressible Navier Stokes equations");

  // It can be useful to define the mixture materials with a fixed phase fraction instead
  // of solving the equations
  params.addParam<bool>("add_scalar_equation", true, "");
  params.renameParam("add_scalar_equation",
                     "add_phase_transport_equation",
                     "Whether to add the phase transport equation.");

  // The flow physics is obtained from the scalar transport base class
  // The fluid heat transfer physics is retrieved even if unspecified
  params.addParam<PhysicsName>(
      "fluid_heat_transfer_physics",
      "NavierStokesFV",
      "WCNSFVFluidHeatTransferPhysics generating the fluid energy equation");

  params.addParam<bool>(
      "use_external_mixture_properties",
      false,
      "Whether to use the simple NSFVMixtureFunctorMaterial or use a more complex model "
      "defined outside of the Physics");
  params.addParam<bool>("output_all_properties",
                        false,
                        "Whether to output every functor material property defined to Exodus");

  params.renameParam("initial_scalar_variables",
                     "initial_phase_fraction",
                     "Initial value of the main phase fraction variable");
  params.renameParam("passive_scalar_diffusivity",
                     "phase_fraction_diffusivity",
                     "Functor names for the diffusivities used for the main phase fraction.");

  // Phase change parameters
  params.addParam<MooseFunctorName>(
      NS::alpha_exchange, 0, "Name of the volumetric phase exchange coefficient");
  params.addParam<bool>("add_phase_change_energy_term",
                        false,
                        "Whether to add a phase change term based on the latent heat of fusion in "
                        "the energy equation");

  // Drift flux model parameters
  params.addParam<bool>("add_drift_flux_momentum_terms",
                        false,
                        "Whether to add the drift flux terms to the momentum equation");
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>("density_interp_method",
                             coeff_interp_method,
                             "Face interpolation method for the density in the drift flux term.");
  params.addParam<bool>(
      "add_advection_slip_term", false, "Whether to use the advection-slip model");
  params.addParam<MooseFunctorName>(
      "slip_linear_friction_name",
      "Name of the functor providing the scalar linear friction coefficient");

  // Properties of the first phase (can be a liquid or a gas)
  params.addRequiredParam<MooseFunctorName>(
      "phase_1_fraction_name",
      "Name of the first phase fraction variable, it will be created as a functor material "
      "property if it does not exist already.");
  params.addRequiredParam<MooseFunctorName>("phase_1_density_name",
                                            "Name of the density functor for phase 1");
  params.addRequiredParam<MooseFunctorName>("phase_1_viscosity_name",
                                            "Name of the viscosity functor for phase 1");
  params.addRequiredParam<MooseFunctorName>("phase_1_specific_heat_name",
                                            "Name of the specific heat functor for phase 1");
  params.addRequiredParam<MooseFunctorName>("phase_1_thermal_conductivity_name",
                                            "Name of the thermal conductivity functor for phase 1");

  // Properties of phase 2 (can be solid, another liquid, or gaseous)
  params.renameParam("passive_scalar_names",
                     "phase_2_fraction_name",
                     "Name of the second phase fraction variable (can be a dispersed phase)");
  params.addRequiredParam<MooseFunctorName>("phase_2_density_name",
                                            "Name of the density functor for phase 2");
  params.addRequiredParam<MooseFunctorName>("phase_2_viscosity_name",
                                            "Name of the viscosity functor for phase 2");
  params.addRequiredParam<MooseFunctorName>("phase_2_specific_heat_name",
                                            "Name of the specific heat functor for phase 2");
  params.addRequiredParam<MooseFunctorName>("phase_2_thermal_conductivity_name",
                                            "Name of the thermal conductivity functor for phase 2");

  // Dispersed phase properties
  params.addParam<MooseFunctorName>(
      "particle_diameter", 1, "Particle size if using a dispersed phase");
  params.addParam<bool>("use_dispersed_phase_drag_model",
                        false,
                        "Adds a linear friction term with the dispersed phase drag model");

  // Not applicable currently
  params.suppressParameter<std::vector<MooseFunctorName>>("passive_scalar_source");
  params.suppressParameter<std::vector<std::vector<MooseFunctorName>>>(
      "passive_scalar_coupled_source");
  params.suppressParameter<std::vector<std::vector<Real>>>("passive_scalar_coupled_source_coeff");

  // Boundary conditions
  params.renameParam("passive_scalar_inlet_types",
                     "phase_fraction_inlet_type",
                     "Types for the inlet boundary for the phase fraction.");
  params.renameParam("passive_scalar_inlet_functors",
                     "phase_fraction_inlet_functors",
                     "Functors describing the inlet phase fraction boundary condition.");

  // Spatial finite volume discretization scheme
  params.renameParam("passive_scalar_advection_interpolation",
                     "phase_advection_interpolation",
                     "The numerical scheme to use for interpolating the phase fraction variable, "
                     "as an advected quantity, to the face.");
  params.renameParam("passive_scalar_face_interpolation",
                     "phase_face_interpolation",
                     "The numerical scheme to interpolate the phase fraction variable to the "
                     "face (separate from the advected quantity interpolation)");
  params.renameParam(
      "passive_scalar_two_term_bc_expansion",
      "phase_two_term_bc_expansion",
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the phase fraction.");

  // Numerical system parameters
  params.renameParam("passive_scalar_scaling",
                     "phase_scaling",
                     "The scaling factor for the phase transport equation");

  // Parameter groups
  params.renameParameterGroup("Passive scalar control", "Mixture transport control");
  params.addParamNamesToGroup("phase_1_density_name phase_1_viscosity_name "
                              "phase_1_specific_heat_name phase_1_thermal_conductivity_name "
                              "phase_2_density_name phase_2_viscosity_name "
                              "phase_2_specific_heat_name phase_2_thermal_conductivity_name "
                              "use_external_mixture_properties",
                              "Mixture material properties");

  params.addParamNamesToGroup("fluid_heat_transfer_physics " + NS::alpha_exchange +
                                  " add_phase_change_energy_term",
                              "Phase change");
  params.addParamNamesToGroup("add_drift_flux_momentum_terms density_interp_method",
                              "Drift flux model");
  params.addParamNamesToGroup("add_advection_slip_term", "Advection slip model");

  return params;
}

WCNSFVTwoPhaseMixturePhysics::WCNSFVTwoPhaseMixturePhysics(const InputParameters & parameters)
  : WCNSFVScalarTransportPhysics(parameters),
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
    _fluid_energy_physics = getCoupledPhysics<WCNSFVFluidHeatTransferPhysics>(
        getParam<PhysicsName>("fluid_heat_transfer_physics"), true);
    // Check for a missing parameter / do not support isolated physics for now
    if (!_fluid_energy_physics &&
        !getCoupledPhysics<const WCNSFVFluidHeatTransferPhysics>(true).empty())
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
      mooseError("Density name should for Physics '",
                 _fluid_energy_physics->name(),
                 "' should be 'rho_mixture'");
    if (_fluid_energy_physics->getSpecificHeatName() != "cp_mixture")
      mooseError("Specific heat name should for Physics '",
                 _fluid_energy_physics->name(),
                 "' should be 'cp_mixture'");
  }
  if (_flow_equations_physics)
  {
    if (_flow_equations_physics->densityName() != "rho_mixture")
      mooseError("Density name should for Physics ,",
                 _flow_equations_physics->name(),
                 "' should be 'rho_mixture'");
  }

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
}

void
WCNSFVTwoPhaseMixturePhysics::addFVKernels()
{
  WCNSFVScalarTransportPhysics::addFVKernels();

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
WCNSFVTwoPhaseMixturePhysics::setSlipVelocityParams(InputParameters & params) const
{
  params.set<MooseFunctorName>("u_slip") = "vel_slip_x";
  if (dimension() >= 2)
    params.set<MooseFunctorName>("v_slip") = "vel_slip_y";
  if (dimension() >= 3)
    params.set<MooseFunctorName>("w_slip") = "vel_slip_z";
}

void
WCNSFVTwoPhaseMixturePhysics::addPhaseInterfaceTerm()
{
  auto params = getFactory().getValidParams("NSFVMixturePhaseInterface");
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _phase_2_fraction_name;
  params.set<MooseFunctorName>("phase_coupled") = _phase_1_fraction_name;
  params.set<MooseFunctorName>("alpha") = getParam<MooseFunctorName>(NS::alpha_exchange);
  getProblem().addFVKernel("NSFVMixturePhaseInterface", prefix() + "phase_interface", params);
}

void
WCNSFVTwoPhaseMixturePhysics::addPhaseChangeEnergySource()
{
  auto params = getFactory().getValidParams("NSFVPhaseChangeSource");
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _fluid_energy_physics->getFluidTemperatureName();
  params.set<MooseFunctorName>("liquid_fraction") = _phase_1_fraction_name;
  params.set<MooseFunctorName>("L") = NS::latent_heat;
  params.set<MooseFunctorName>(NS::density) = "rho_mixture";
  params.set<MooseFunctorName>("T_solidus") = NS::T_solidus;
  params.set<MooseFunctorName>("T_liquidus") = NS::T_liquidus;
  getProblem().addFVKernel("NSFVPhaseChangeSource", prefix() + "phase_change_energy", params);

  // TODO add phase equation source term corresponding to this term
}

void
WCNSFVTwoPhaseMixturePhysics::addPhaseDriftFluxTerm()
{
  const std::vector<std::string> components = {"x", "y", "z"};
  for (const auto dim : make_range(dimension()))
  {
    auto params = getFactory().getValidParams("WCNSFV2PMomentumDriftFlux");
    assignBlocks(params, _blocks);
    params.set<NonlinearVariableName>("variable") =
        _flow_equations_physics->getVelocityNames()[dim];
    params.set<MooseFunctorName>("u_slip") = "vel_slip_x";
    if (dimension() >= 2)
      params.set<MooseFunctorName>("v_slip") = "vel_slip_y";
    if (dimension() >= 3)
      params.set<MooseFunctorName>("w_slip") = "vel_slip_z";
    params.set<MooseFunctorName>("rho_d") = _phase_2_density;
    params.set<MooseFunctorName>("fraction_dispersed") = _phase_2_fraction_name;
    params.set<MooseEnum>("momentum_component") = components[dim];
    params.set<MooseEnum>("density_interp_method") = getParam<MooseEnum>("density_interp_method");
    params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
    getProblem().addFVKernel(
        "WCNSFV2PMomentumDriftFlux", prefix() + "drift_flux_" + components[dim], params);
  }
}

void
WCNSFVTwoPhaseMixturePhysics::addAdvectionSlipTerm()
{
  const std::vector<std::string> components = {"x", "y", "z"};
  for (const auto dim : make_range(dimension()))
  {
    auto params = getFactory().getValidParams("WCNSFV2PMomentumAdvectionSlip");
    assignBlocks(params, _blocks);
    params.set<NonlinearVariableName>("variable") =
        _flow_equations_physics->getVelocityNames()[dim];
    params.set<MooseFunctorName>("u_slip") = "vel_slip_x";
    if (dimension() >= 2)
      params.set<MooseFunctorName>("v_slip") = "vel_slip_y";
    if (dimension() >= 3)
      params.set<MooseFunctorName>("w_slip") = "vel_slip_z";
    params.set<MooseFunctorName>(NS::density) = _phase_1_density;
    params.set<MooseFunctorName>("rho_d") = _phase_2_density;
    params.set<MooseFunctorName>("fraction_dispersed") = _phase_2_fraction_name;
    params.set<MooseEnum>("momentum_component") = components[dim];
    params.set<MooseEnum>("advected_interp_method") =
        _flow_equations_physics->getMomentumFaceInterpolationMethod();
    params.set<MooseEnum>("velocity_interp_method") =
        _flow_equations_physics->getVelocityFaceInterpolationMethod();
    params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
    getProblem().addFVKernel(
        "WCNSFV2PMomentumAdvectionSlip", prefix() + "advection_slip_" + components[dim], params);
  }
}

void
WCNSFVTwoPhaseMixturePhysics::addMaterials()
{
  // Add the phase fraction variable, for output purposes mostly
  if (!getProblem().hasFunctor(_phase_1_fraction_name, /*thread_id=*/0))
  {
    auto params = getFactory().getValidParams("ADParsedFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<std::string>("expression") = "1 - " + _phase_2_fraction_name;
    params.set<std::vector<std::string>>("functor_names") = {_phase_2_fraction_name};
    params.set<std::string>("property_name") = _phase_1_fraction_name;
    params.set<std::vector<std::string>>("output_properties") = {_phase_1_fraction_name};
    params.set<std::vector<OutputName>>("outputs") = {"all"};
    getProblem().addMaterial("ADParsedFunctorMaterial", prefix() + "phase_1_fraction", params);

    // One of the phase fraction should exist though (either as a variable or set by a
    // NSLiquidFractionAux)
    if (!getProblem().hasFunctor(_phase_2_fraction_name, /*thread_id=*/0))
      paramError("Phase 2 fraction should be defined as a variable or auxiliary variable");
  }
  if (!getProblem().hasFunctor(_phase_2_fraction_name, /*thread_id=*/0))
  {
    auto params = getFactory().getValidParams("ADParsedFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<std::string>("expression") = "1 - " + _phase_1_fraction_name;
    params.set<std::vector<std::string>>("functor_names") = {_phase_1_fraction_name};
    params.set<std::string>("property_name") = _phase_2_fraction_name;
    params.set<std::vector<std::string>>("output_properties") = {_phase_2_fraction_name};
    params.set<std::vector<OutputName>>("outputs") = {"all"};
    getProblem().addMaterial("ADParsedFunctorMaterial", prefix() + "phase_2_fraction", params);
  }

  // Compute mixture properties
  if (!_use_external_mixture_properties)
  {
    auto params = getFactory().getValidParams("NSFVMixtureFunctorMaterial");
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
    getProblem().addMaterial("NSFVMixtureFunctorMaterial", prefix() + "mixture_material", params);
  }

  // Compute slip terms as functors, used by the drift flux kernels
  if (_use_advection_slip || _use_drift_flux || _add_phase_equation)
  {
    const std::vector<std::string> vel_components = {"u", "v", "w"};
    const std::vector<std::string> components = {"x", "y", "z"};
    for (const auto dim : make_range(dimension()))
    {
      auto params = getFactory().getValidParams("WCNSFV2PSlipVelocityFunctorMaterial");
      assignBlocks(params, _blocks);
      params.set<MooseFunctorName>("slip_velocity_name") = "vel_slip_" + components[dim];
      params.set<MooseEnum>("momentum_component") = components[dim];
      for (const auto j : make_range(dimension()))
        params.set<std::vector<VariableName>>(vel_components[j]) = {
            _flow_equations_physics->getVelocityNames()[j]};
      params.set<MooseFunctorName>(NS::density) = _phase_1_density;
      params.set<MooseFunctorName>(NS::mu) = "mu_mixture";
      params.set<MooseFunctorName>("rho_d") = _phase_2_density;
      params.set<RealVectorValue>("gravity") = _flow_equations_physics->gravityVector();
      if (isParamValid("slip_linear_friction_name"))
        params.set<MooseFunctorName>("linear_coef_name") =
            getParam<MooseFunctorName>("slip_linear_friction_name");
      else if (getParam<bool>("use_dispersed_phase_drag_model"))
        params.set<MooseFunctorName>("linear_coef_name") = "Darcy_coefficient";
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
                   "WCNSFV2PSlipVelocityFunctorMaterial created by this Physics required a scalar "
                   "field linear friction factor.");
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
      getProblem().addMaterial(
          "WCNSFV2PSlipVelocityFunctorMaterial", prefix() + "slip_" + components[dim], params);
    }
  }

  // Add a default drag model for a dispersed phase
  if (getParam<bool>("use_dispersed_phase_drag_model"))
  {
    const std::vector<std::string> vel_components = {"u", "v", "w"};

    auto params = getFactory().getValidParams("NSFVDispersePhaseDragFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>("drag_coef_name") = "Darcy_coefficient";
    for (const auto j : make_range(dimension()))
      params.set<MooseFunctorName>(vel_components[j]) = {
          _flow_equations_physics->getVelocityNames()[j]};
    params.set<MooseFunctorName>(NS::density) = "rho_mixture";
    params.set<MooseFunctorName>(NS::mu) = "mu_mixture";
    params.set<MooseFunctorName>("particle_diameter") =
        getParam<MooseFunctorName>("particle_diameter");
    if (getParam<bool>("output_all_properties"))
      params.set<std::vector<OutputName>>("outputs") = {"all"};
    getProblem().addMaterial(
        "NSFVDispersePhaseDragFunctorMaterial", prefix() + "dispersed_drag", params);
  }
}
