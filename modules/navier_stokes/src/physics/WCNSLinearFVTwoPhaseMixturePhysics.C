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

namespace
{
/// Parameter names of the velocity components on the objects that take them one by one
const std::string velocity_components[3] = {"u", "v", "w"};
}

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

  params.addParam<MooseFunctorName>(
      "interfacial_latent_heat",
      "Latent heat of the transition the interfacial mass transfer represents, per unit mass. "
      "Supplied together with 'interfacial_mass_transfer' to place the energy that transfer "
      "carries into the energy equation. Available from a TwoPhaseFluidProperties object as "
      "h_lat(p, T), which is why it is taken as a functor rather than a constant.");
  params.addParam<MooseFunctorName>(
      "interfacial_mass_transfer",
      "Rate at which mass is transferred into the dispersed phase per unit mixture volume, the "
      "Gamma of the dispersed phase mass balance, prescribed rather than closed. Positive "
      "generates "
      "the dispersed phase. Supplying it directly lets the coupling be exercised against a rate "
      "with an analytic answer; supply 'interfacial_area' and 'T_saturation' instead to have the "
      "rate computed from the transported area. Note that no counterpart is needed in the mixture "
      "mass or momentum equations: mass and momentum are conserved in the exchange, and the volume "
      "the phase change creates reaches the pressure equation through the phase fraction, by way "
      "of 'add_mass_density_transient'.");
  params.addParam<MooseFunctorName>(
      "interfacial_area",
      "Interfacial area concentration of the dispersed phase, normally the variable of an "
      "interfacial area transport equation. Supplying it together with 'T_saturation' and "
      "'interfacial_latent_heat' closes the interfacial mass transfer rate on the solved area "
      "instead of taking it as prescribed, so that the phase equation and the area equation are "
      "driven by one rate rather than two independently supplied ones. The computed rate is "
      "declared as '<name of this Physics block>_interfacial_mass_transfer_rate', which is the "
      "name to give the 'mass_transfer_rate' of the interfacial area source and sink so that it "
      "reads the same rate the phase equation is given.");
  params.addParam<MooseFunctorName>(
      "T_saturation",
      "Saturation temperature of the transition the interfacial mass transfer represents. Supplied "
      "together with 'interfacial_area' to close that transfer. Available from a "
      "TwoPhaseFluidProperties object as T_sat(p), which is why it is taken as a functor.");
  params.addParamNamesToGroup(
      "interfacial_latent_heat interfacial_mass_transfer interfacial_area T_saturation",
      "Phase change");
  params.addParam<MooseFunctorName>(
      "phase_1_density_time_derivative",
      "Time derivative of the continuous phase density. Optional counterpart of "
      "'phase_2_density_time_derivative'. It is zero for every case reported so far, but the "
      "formulation does not assume it.");
  params.addParam<bool>(
      "add_mass_density_transient",
      false,
      "Whether to add the storage term d(rho_m)/dt to the pressure equation. Mixture continuity is "
      "d(rho_m)/dt + div(rho_m u_m) = 0, and the pressure equation of the segregated algorithm "
      "realises the second term alone, which is exact in a steady state and not otherwise. Setting "
      "this lets a cell accumulate mass rather than pass a divergence free mass flux at every "
      "instant, which is what a compressible SIMPLE does and an incompressible one cannot. It is "
      "the only addition mixture continuity needs: the pressure equation is assembled on the "
      "mass flux rho_m u_m, so the dilatation the relative motion of the phases produces is "
      "already inside its divergence.");
  params.addParam<MooseFunctorName>(
      "phase_2_density_time_derivative",
      "Time derivative of the dispersed phase density, needed by the mixture density storage term "
      "of 'add_mass_density_transient' and by nothing else. The phase equation does not need it: "
      "its LinearFVTimeDerivative is assembled in conservative form and takes the whole of "
      "d(rho_d alpha)/dt from the multiplier's own state history. Leaving this unset asserts that "
      "the dispersed phase density does not change in time.");
  params.addParam<MooseFunctorName>(
      "phase_2_density_pressure_derivative",
      "Partial derivative of the dispersed phase density with respect to pressure, at fixed phase "
      "fraction. Supply this whenever 'phase_2_density_name' is read from the solved pressure. It "
      "is what lets the pressure driven part of d(rho_m)/dt be assembled implicitly rather than "
      "lagged. Supply the compressibility of the phase itself: for an isothermal ideal gas it is "
      "rho_d / p_absolute. The factor of rho_c/rho_d by which the phase equation's conservative "
      "form amplifies it is applied here rather than asked for. Without this the pressure driven "
      "part rides on the right hand side, and the outer iteration of the segregated solve stops "
      "contracting at high void fraction or small time step. Optional: omitted, the term is "
      "assembled explicitly exactly as before.");
  params.addParam<MooseFunctorName>(
      "phase_1_density_pressure_derivative",
      "Partial derivative of the continuous phase density with respect to pressure. Optional "
      "counterpart of 'phase_2_density_pressure_derivative'; it is zero for every case reported "
      "so far, the continuous phase being a liquid.");

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
    _use_advection_slip(getParam<bool>("add_advection_slip_term")),
    _close_mass_transfer_on_area(isParamValid("interfacial_area")),
    // The one name every consumer of the transfer rate reads, whether it was prescribed or closed
    // on the area. Resolving it once here is what keeps the phase equation, the latent heat sink
    // and the interfacial area source from being pointed at different rates.
    _interfacial_mass_transfer(_close_mass_transfer_on_area
                                   ? MooseFunctorName(prefix() + "interfacial_mass_transfer_rate")
                                   : (isParamValid("interfacial_mass_transfer")
                                          ? getParam<MooseFunctorName>("interfacial_mass_transfer")
                                          : MooseFunctorName()))
{
  if (_close_mass_transfer_on_area)
  {
    if (isParamValid("interfacial_mass_transfer"))
      paramError("interfacial_mass_transfer",
                 "The transfer rate cannot be both prescribed and closed on the interfacial area. "
                 "Supply this parameter to prescribe it, or 'interfacial_area' to close it.");
    if (!isParamValid("T_saturation"))
      paramError("T_saturation",
                 "Closing the interfacial mass transfer on the interfacial area needs a saturation "
                 "temperature for the transfer to be driven by");
    if (!isParamValid("interfacial_latent_heat"))
      paramError("interfacial_latent_heat",
                 "Closing the interfacial mass transfer on the interfacial area needs the latent "
                 "heat of the transition, which sets the mass converted per unit of heat");
  }

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

  // The area-closed transfer is driven by the departure of the mixture temperature from
  // saturation, so there has to be an energy equation for that temperature to come from. A
  // prescribed rate does not need one, which is why this is checked only on that path.
  if (_close_mass_transfer_on_area && !_has_energy_equation)
    paramError("interfacial_area",
               "Closing the interfacial mass transfer on the interfacial area needs an energy "
               "equation, since the transfer is driven by the departure of the mixture temperature "
               "from saturation. Set 'fluid_heat_transfer_physics'.");

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
  // The phase change term is 'coefficient * dT/dt', so it must be added to the equation that
  // actually solves for the temperature. When the energy physics solves for enthalpy instead,
  // the temperature is only an auxiliary variable and the term cannot be assembled as written.
  if (_has_energy_equation && getParam<bool>("add_phase_change_energy_term") &&
      _fluid_energy_physics->parameters().get<bool>("solve_for_enthalpy"))
    paramError("add_phase_change_energy_term",
               "The phase change energy term is currently only implemented for an energy equation "
               "solved for the temperature. Physics '",
               _fluid_energy_physics->name(),
               "' is solving for the enthalpy instead.");
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

  if (_flow_equations_physics && _flow_equations_physics->hasFlowEquations() &&
      getParam<bool>("add_mass_density_transient"))
    addMassDensityTransientTerm();

  if (_add_phase_equation && !_interfacial_mass_transfer.empty())
    addInterfacialMassTransferTerm();

  if (_has_energy_equation && !_interfacial_mass_transfer.empty() &&
      isParamValid("interfacial_latent_heat"))
    addLatentHeatTransferTerm();

  if (_fluid_energy_physics && _fluid_energy_physics->hasEnergyEquation() &&
      getParam<bool>("add_phase_change_energy_term"))
    addPhaseChangeEnergySource();

  if (_fluid_energy_physics && _fluid_energy_physics->hasEnergyEquation() &&
      _fluid_energy_physics->getParam<bool>("include_pressure_work"))
    addEnergyPressureWorkDriftTerm();

  if (_flow_equations_physics && _flow_equations_physics->hasFlowEquations() && _use_drift_flux)
  {
    addPhaseDriftFluxTerm();
    // The enthalpy carried by the relative motion is the energy counterpart of the diffusion
    // stress, so it belongs wherever the diffusion stress does
    if (_has_energy_equation)
      addPhaseEnergyDriftFluxTerm();
  }
  if (_flow_equations_physics && _flow_equations_physics->hasFlowEquations() && _use_advection_slip)
    addAdvectionSlipTerm();
}

MooseFunctorName
WCNSLinearFVTwoPhaseMixturePhysics::scalarConservativeDensity(const VariableName & vname) const
{
  // Only the phase fraction is transported as a mass. Any other scalar carried by this Physics is
  // solved for itself.
  return (vname == _phase_2_fraction_name) ? _phase_2_density : MooseFunctorName();
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
  params.set<MooseFunctorName>("rho_d") = _phase_2_density;
  params.set<MooseFunctorName>("rho_c") = _phase_1_density;
  params.set<MooseFunctorName>("fraction_dispersed") = _phase_2_fraction_name;
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addLatentHeatTransferTerm()
{
  // Generating the dispersed phase absorbs its latent heat from the mixture, so the energy equation
  // takes a sink of Gamma * h_lat, and a source of the same size when the transfer runs the other
  // way. Wu et al. carry this as -h'_l Gamma_g in a liquid phase enthalpy equation. This is a
  // single energy equation for the mixture, so what the term supports is a transfer at, or close
  // to, thermal equilibrium.
  const auto latent_source = prefix() + "latent_heat_source";
  addParsedFunctorMaterial(
      latent_source + "_mat",
      latent_source,
      "gamma * h_lat",
      {_interfacial_mass_transfer, getParam<MooseFunctorName>("interfacial_latent_heat")},
      {"gamma", "h_lat"},
      false);
  {
    auto params = getFactory().getValidParams("LinearFVSource");
    assignBlocks(params, _blocks);
    // The solved energy variable, not the temperature: with 'solve_for_enthalpy' the temperature
    // is not a solver variable, and a source placed on it would target the wrong equation. The
    // term itself is the same either way, a power per unit volume, because both forms of the
    // energy equation carry their sources in those units.
    params.set<LinearVariableName>("variable") =
        _fluid_energy_physics->getFluidEnergyVariableName();
    params.set<MooseFunctorName>("source_density") = latent_source;
    // A sink: the energy leaves the mixture with the phase that is generated
    params.set<MooseFunctorName>("scaling_factor") = "-1";
    getProblem().addLinearFVKernel("LinearFVSource", prefix() + "latent_heat", params);
  }
}

MooseFunctorName
WCNSLinearFVTwoPhaseMixturePhysics::buildMixtureDensityTimeDerivative()
{
  // Differentiating rho_m = alpha rho_d + (1 - alpha) rho_c,
  //
  //   d(rho_m)/dt = (rho_d - rho_c) d(alpha)/dt + alpha d(rho_d)/dt + (1 - alpha) d(rho_c)/dt ,
  //
  // which assumes nothing about either phase density. The phase fraction derivative comes from the
  // variable's own dot(), which MooseLinearVariableFV builds from the time integrator's
  // coefficients: it is therefore the same discrete operator LinearFVTimeDerivative assembles in
  // the phase equation, so the two cannot disagree. The density derivatives are supplied by the
  // user and default to zero.
  const auto drho_m_dt = prefix() + "drho_m_dt";
  if (_built_drho_m_dt)
    return drho_m_dt;

  const auto alpha_dot = prefix() + "alpha_dot";
  {
    auto params = getFactory().getValidParams("GenericFunctorTimeDerivativeMaterial");
    assignBlocks(params, _blocks);
    params.set<std::vector<std::string>>("prop_names") = {alpha_dot};
    params.set<std::vector<MooseFunctorName>>("prop_values") = {_phase_2_fraction_name};
    getProblem().addMaterial("GenericFunctorTimeDerivativeMaterial", alpha_dot + "_mat", params);
  }
  addParsedFunctorMaterial(
      drho_m_dt + "_mat",
      drho_m_dt,
      "(rho_d - rho_c) * alpha_dot + alpha * drho_d_dt + (1 - alpha) * drho_c_dt",
      {_phase_2_density,
       _phase_1_density,
       alpha_dot,
       _phase_2_fraction_name,
       isParamValid("phase_2_density_time_derivative")
           ? getParam<MooseFunctorName>("phase_2_density_time_derivative")
           : MooseFunctorName("0"),
       isParamValid("phase_1_density_time_derivative")
           ? getParam<MooseFunctorName>("phase_1_density_time_derivative")
           : MooseFunctorName("0")},
      {"rho_d", "rho_c", "alpha_dot", "alpha", "drho_d_dt", "drho_c_dt"},
      false);

  _built_drho_m_dt = true;
  return drho_m_dt;
}

MooseFunctorName
WCNSLinearFVTwoPhaseMixturePhysics::buildMixtureDensityPressureDerivative()
{
  // How the mixture density responds to pressure, the coefficient the pressure driven part of the
  // storage term carries onto the matrix diagonal. Not d(rho_m)/dp at fixed phase fraction: the
  // phase equation is assembled in conservative form and holds the dispersed phase mass
  // m_d = rho_d alpha, so raising the pressure compresses the dispersed phase and
  //
  //   d(alpha)/dp = -(alpha / rho_d) d(rho_d)/dp ,
  //
  // which substituted into rho_m = m_d + (1 - alpha) rho_c, the first group held fixed, leaves
  //
  //   d(rho_m)/dp = (alpha rho_c / rho_d) d(rho_d)/dp + (1 - alpha) d(rho_c)/dp .
  //
  // The ratio rho_c/rho_d is over eight hundred for air in water, so the coefficient reaches two
  // hundred at a void fraction of a quarter. The same quantity is subtracted from the explicit
  // source, so it cancels at convergence and cannot change the converged answer.
  //
  // The phase compressibilities are supplied rather than differentiated, the densities reaching
  // this Physics as opaque functors.
  const auto drho_m_dp = prefix() + "drho_m_dp";
  if (_built_drho_m_dp)
    return drho_m_dp;

  addParsedFunctorMaterial(drho_m_dp + "_mat",
                           drho_m_dp,
                           "alpha * rho_c / rho_d * drho_d_dp + (1 - alpha) * drho_c_dp",
                           {_phase_2_fraction_name,
                            _phase_1_density,
                            _phase_2_density,
                            isParamValid("phase_2_density_pressure_derivative")
                                ? getParam<MooseFunctorName>("phase_2_density_pressure_derivative")
                                : MooseFunctorName("0"),
                            isParamValid("phase_1_density_pressure_derivative")
                                ? getParam<MooseFunctorName>("phase_1_density_pressure_derivative")
                                : MooseFunctorName("0")},
                           {"alpha", "rho_c", "rho_d", "drho_d_dp", "drho_c_dp"},
                           false);

  _built_drho_m_dp = true;
  return drho_m_dp;
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addInterfacialMassTransferRateMaterial()
{
  // The rate is declared once, here, and read by every equation that needs it: the phase equation
  // source below, the latent heat sink on the energy equation, and the phase change term of the
  // interfacial area transport equation. Those three cannot disagree about it because there is one
  // functor rather than one parameter per consumer.
  auto params = getFactory().getValidParams("WCNSFV2PInterfacialMassTransferFunctorMaterial");
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>("interfacial_area") = getParam<MooseFunctorName>("interfacial_area");
  params.set<MooseFunctorName>("fraction_dispersed") = _phase_2_fraction_name;
  params.set<MooseFunctorName>(NS::T_fluid) = _fluid_energy_physics->getFluidTemperatureName();
  params.set<MooseFunctorName>("T_saturation") = getParam<MooseFunctorName>("T_saturation");
  params.set<MooseFunctorName>("latent_heat") =
      getParam<MooseFunctorName>("interfacial_latent_heat");
  params.set<MooseFunctorName>("rho_c") = _phase_1_density;
  params.set<MooseFunctorName>("mu_c") = _phase_1_viscosity;
  params.set<MooseFunctorName>("k_c") = _phase_1_thermal_conductivity;
  setSlipVelocityParams(params);
  params.set<MooseFunctorName>("interfacial_mass_transfer_name") = _interfacial_mass_transfer;
  getProblem().addMaterial("WCNSFV2PInterfacialMassTransferFunctorMaterial",
                           _interfacial_mass_transfer + "_mat",
                           params);
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addInterfacialMassTransferTerm()
{
  // The dispersed phase mass balance is
  //
  //   d(rho_d alpha)/dt + div(rho_d alpha u_d) - div(rho_d D grad(alpha)) = Gamma ,
  //
  // so the transfer enters as a source on that equation and nowhere else in the mass or momentum
  // equations. The volume the exchange creates reaches the pressure equation through d(rho_m)/dt,
  // which is why 'add_mass_density_transient' is a prerequisite in a flowing case.
  auto params = getFactory().getValidParams("LinearFVSource");
  assignBlocks(params, _blocks);
  params.set<LinearVariableName>("variable") = _phase_2_fraction_name;
  params.set<MooseFunctorName>("source_density") = _interfacial_mass_transfer;
  params.set<MooseFunctorName>("scaling_factor") = "1";
  getProblem().addLinearFVKernel("LinearFVSource", prefix() + "interfacial_mass_transfer", params);
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addMassDensityTransientTerm()
{
  // Mixture continuity is d(rho_m)/dt + div(rho_m u_m) = 0. The pressure equation realises the
  // divergence alone, so this supplies the storage term, built by
  // buildMixtureDensityTimeDerivative().
  const auto drho_m_dt = buildMixtureDensityTimeDerivative();

  // Where the dispersed phase density is read from the solved pressure, part of d(rho_m)/dt is a
  // function of the very unknown this equation solves for:
  //
  //   d(rho_m)/dt = R + (d(rho_m)/dp) dp/dt ,   R = (rho_d - rho_c) d(alpha)/dt + ... ,
  //
  // and a LinearFVSource contributes nothing to the matrix, so lagging that second piece one outer
  // iteration gives the segregated loop a gain of order (d(rho_m)/dp) V / (a_P dt), which stops it
  // contracting above unity. LinearFVTimeDerivative in its non-conservative form is c du/dt with c
  // on the diagonal, so handing it the pressure and d(rho_m)/dp puts the coefficient on the matrix
  // and leaves R in the explicit source. The two forms have the same fixed point, the piece removed
  // from one being the piece added to the other.
  //
  // This needs a phase compressibility, which arrives as an opaque functor and cannot be inferred.
  // Either phase's will do, though in practice it is the dispersed one that matters. With neither,
  // the explicit assembly is kept.
  const bool implicit_pressure_part = isParamValid("phase_2_density_pressure_derivative") ||
                                      isParamValid("phase_1_density_pressure_derivative");
  MooseFunctorName source_density = drho_m_dt;

  if (implicit_pressure_part)
  {
    const auto drho_m_dp = buildMixtureDensityPressureDerivative();
    const auto & pressure_name = _flow_equations_physics->getPressureName();

    // dp/dt, built here rather than taken from the user so that the piece subtracted from the
    // source is the same discrete operator LinearFVTimeDerivative puts on the matrix.
    const auto p_dot = prefix() + "pressure_dot";
    {
      auto params = getFactory().getValidParams("GenericFunctorTimeDerivativeMaterial");
      assignBlocks(params, _blocks);
      params.set<std::vector<std::string>>("prop_names") = {p_dot};
      params.set<std::vector<MooseFunctorName>>("prop_values") = {pressure_name};
      getProblem().addMaterial("GenericFunctorTimeDerivativeMaterial", p_dot + "_mat", params);
    }

    // R = d(rho_m)/dt - (d(rho_m)/dp) dp/dt, the part that is genuinely explicit.
    source_density = prefix() + "drho_m_dt_explicit";
    addParsedFunctorMaterial(source_density + "_mat",
                             source_density,
                             "drho_m_dt - drho_m_dp * p_dot",
                             {drho_m_dt, drho_m_dp, p_dot},
                             {"drho_m_dt", "drho_m_dp", "p_dot"},
                             false);

    // The pressure driven part, implicit. Non-conservative form: the coefficient is a partial
    // derivative held outside the time derivative, not a density being transported, so
    // d(coefficient p)/dt is not what is wanted here.
    {
      auto params = getFactory().getValidParams("LinearFVTimeDerivative");
      assignBlocks(params, _blocks);
      params.set<LinearVariableName>("variable") = pressure_name;
      params.set<MooseFunctorName>("factor") = drho_m_dp;
      params.set<bool>("conservative_form") = false;
      getProblem().addLinearFVKernel(
          "LinearFVTimeDerivative", prefix() + "mass_density_transient_implicit", params);
    }
  }

  {
    // Sign. LinearFVDivergence and LinearFVSource do not share a sign convention on the right hand
    // side, so the factor is negative although
    //   div(rho_m Ainv grad p) = div(rho_m HbyA) + d(rho_m)/dt
    // suggests otherwise. A positive factor doubles the mass imbalance instead of closing it; see
    // mass-balance-transient.i.
    auto params = getFactory().getValidParams("LinearFVSource");
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _flow_equations_physics->getPressureName();
    params.set<MooseFunctorName>("source_density") = source_density;
    params.set<MooseFunctorName>("scaling_factor") = "-1";
    getProblem().addLinearFVKernel("LinearFVSource", prefix() + "mass_density_transient", params);
  }
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addPhaseInterfaceTerm()
{
  // The phase equation is assembled in conservative form, so the exchange term is scaled by the
  // dispersed phase density along with every other term of that equation. Built as a functor
  // rather than folded into the coefficient so that a non-uniform density is handled correctly.
  const auto scaled_exchange = prefix() + "phase_exchange_coeff";
  addParsedFunctorMaterial(scaled_exchange + "_mat",
                           scaled_exchange,
                           "rho_d_exchange * alpha_exchange_coeff",
                           {_phase_2_density, getParam<MooseFunctorName>(NS::alpha_exchange)},
                           {"rho_d_exchange", "alpha_exchange_coeff"},
                           false);
  {
    auto params = getFactory().getValidParams("LinearFVReaction");
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _phase_2_fraction_name;
    params.set<MooseFunctorName>("coeff") = scaled_exchange;
    getProblem().addLinearFVKernel(
        "LinearFVReaction", prefix() + "phase_interface_reaction", params);
  }
  {
    auto params = getFactory().getValidParams("LinearFVSource");
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _phase_2_fraction_name;
    params.set<MooseFunctorName>("source_density") = _phase_1_fraction_name;
    params.set<MooseFunctorName>("scaling_factor") = scaled_exchange;
    getProblem().addLinearFVKernel("LinearFVSource", prefix() + "phase_interface_source", params);
  }
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addPhaseChangeCoefficientMaterial()
{
  // Reproduces the coefficient of NSFVPhaseChangeSource, which is
  //
  //   max(6 fl (1 - fl), 0) * L * rho_mixture / (T_liquidus - T_solidus),
  //   fl = (T - T_sol) / (T_liq - T_sol)
  //
  // The 6 is the integral of x (1 - x) between 0 and 1, and the max() clamps the term outside the
  // mushy zone. The liquid fraction is computed from the temperature, as the nonlinear kernel does
  // rather than from its 'liquid_fraction' parameter.
  addParsedFunctorMaterial(
      prefix() + "phase_change_coefficient",
      "phase_change_coefficient",
      "max(6 * ((T - T_sol) / (T_liq - T_sol)) * (1 - ((T - T_sol) / (T_liq - T_sol))), 0) * L * "
      "rho_m / (T_liq - T_sol)",
      {_fluid_energy_physics->getFluidTemperatureName(),
       NS::T_solidus,
       NS::T_liquidus,
       NS::latent_heat,
       "rho_mixture"},
      {"T", "T_sol", "T_liq", "L", "rho_m"},
      getParam<bool>("output_all_properties"));
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addPhaseChangeEnergySource()
{
  // The nonlinear NSFVPhaseChangeSource is 'coefficient * dT/dt', which is the operator
  // LinearFVTimeDerivative assembles when handed a 'factor', with the temporal order of the scheme
  // preserved and the same sign on the left hand side. The coefficient is lagged to the previous
  // fixed point iterate rather than differentiated, so the mushy zone may need tighter fixed point
  // tolerances than the Newton implementation. Assembled against a specific enthalpy it would
  // evaluate c dh/dt, a different quantity, so an enthalpy formulation is refused.
  if (_fluid_energy_physics->solveForEnthalpy())
    paramError("add_phase_change_energy_term",
               "The phase change energy source is a coefficient multiplying the time derivative of "
               "temperature, and the fluid heat transfer Physics is solving for specific enthalpy. "
               "Expressing it in that variable requires the specific heat as a chain rule factor, "
               "which is not implemented. Solve for temperature, or leave this term off.");

  auto params = getFactory().getValidParams("LinearFVTimeDerivative");
  assignBlocks(params, _blocks);
  params.set<LinearVariableName>("variable") = _fluid_energy_physics->getFluidTemperatureName();
  params.set<MooseFunctorName>("factor") = "phase_change_coefficient";
  // Not the conservative form, which the kernel would otherwise take. The coefficient here is not
  // a density being transported, so d(coefficient T)/dt is not a storage term of anything; the
  // term this reproduces is literally the coefficient times the temperature rate, and the
  // coefficient varies in time through the liquid fraction, so the two forms do differ.
  params.set<bool>("conservative_form") = false;
  getProblem().addLinearFVKernel(
      "LinearFVTimeDerivative", prefix() + "phase_change_energy", params);
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addEnergyPressureWorkDriftTerm()
{
  // The energy Physics adds dp/dt + u_m . grad(p). What remains for a mixture is
  // (alpha - c_d) u_s . grad(p) with c_d = alpha rho_d / rho_m, the volumetric drift the slip
  // material declares. The transient part is left to the mixture piece, so dp/dt is counted once.
  const auto drift_velocity = "vel_volumetric_drift_";
  const auto drift_work = prefix() + "pressure_work_drift";
  {
    auto params = getFactory().getValidParams("NSFVPressureWorkFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>(NS::pressure) = _flow_equations_physics->getPressureName();
    params.set<MooseFunctorName>("u") = std::string(drift_velocity) + "x";
    if (dimension() >= 2)
      params.set<MooseFunctorName>("v") = std::string(drift_velocity) + "y";
    if (dimension() >= 3)
      params.set<MooseFunctorName>("w") = std::string(drift_velocity) + "z";
    params.set<bool>("include_time_derivative") = false;
    params.set<MooseFunctorName>("pressure_work_name") = drift_work;
    getProblem().addMaterial("NSFVPressureWorkFunctorMaterial", drift_work + "_mat", params);
  }
  {
    auto params = getFactory().getValidParams("LinearFVSource");
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") =
        _fluid_energy_physics->getFluidEnergyVariableName();
    params.set<MooseFunctorName>("source_density") = drift_work;
    getProblem().addLinearFVKernel(
        "LinearFVSource", prefix() + "energy_pressure_work_drift", params);
  }
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addPhaseDriftFluxTerm()
{
  for (const auto dim : make_range(dimension()))
  {
    const auto object_type = "LinearWCNSFV2PMomentumDriftFlux";
    auto params = getFactory().getValidParams(object_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _flow_equations_physics->getVelocityNames()[dim];
    setRelativeVelocityParams(params);
    params.set<MooseEnum>("momentum_component") = NS::directions[dim];
    params.set<MooseEnum>("density_interp_method") = getParam<MooseEnum>("density_interp_method");
    params.set<UserObjectName>("rhie_chow_user_object") = _flow_equations_physics->rhieChowUOName();
    getProblem().addLinearFVKernel(
        object_type, prefix() + "drift_flux_" + NS::directions[dim], params);
  }
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addPhaseEnergyDriftFluxTerm()
{
  // The term is h_d - h_c = (cp_d - cp_c) T, so it is only assemblable against a temperature
  // variable. When the energy equation solves for the enthalpy the specific heats are not
  // available as a difference multiplying the solution variable.
  if (_fluid_energy_physics->parameters().get<bool>("solve_for_enthalpy"))
    paramError("add_drift_flux_momentum_terms",
               "The enthalpy carried by the relative motion of the phases is currently only "
               "implemented for an energy equation solved for the temperature. Physics '",
               _fluid_energy_physics->name(),
               "' is solving for the enthalpy instead.");

  const auto object_type = "LinearWCNSFV2PEnergyDriftFlux";
  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  params.set<LinearVariableName>("variable") = _fluid_energy_physics->getFluidTemperatureName();
  setRelativeVelocityParams(params);
  params.set<MooseFunctorName>("cp_d") = _phase_2_specific_heat;
  params.set<MooseFunctorName>("cp_c") = _phase_1_specific_heat;
  params.set<MooseEnum>("advected_interp_method") =
      _fluid_energy_physics->parameters().get<MooseEnum>("energy_advection_interpolation");
  // The relative motion carries enthalpy only where the dispersed phase itself may travel, which
  // is the same set of boundaries the phase transport equation uses
  params.set<std::vector<BoundaryName>>("slip_boundaries") = slipBoundaries();
  getProblem().addLinearFVKernel(object_type, prefix() + "energy_drift_flux", params);
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addMixtureSpecificHeatMaterial()
{
  // The mixture energy density is the sum over the phases of each phase's own enthalpy density,
  // rho_m e_m = sum_k a_k rho_k cp_k T, so the specific heat that reproduces it when multiplied by
  // the mixture density is the mass-weighted average, not the volume-weighted average used for the
  // density, the viscosity and the conductivity. See Fluent Theory Guide equation 16.4-7. The
  // phase fraction is clamped to match the mixture property material.
  addParsedFunctorMaterial(
      prefix() + "mixture_specific_heat",
      "cp_mixture",
      "(min(max(fd, 0), 1) * rho_d * cp_d + (1 - min(max(fd, 0), 1)) * rho_c * cp_c) / rho_m",
      {_phase_2_fraction_name,
       _phase_2_density,
       _phase_2_specific_heat,
       _phase_1_density,
       _phase_1_specific_heat,
       "rho_mixture"},
      {"fd", "rho_d", "cp_d", "rho_c", "cp_c", "rho_m"},
      getParam<bool>("output_all_properties"));
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addAdvectionSlipTerm()
{
  mooseError("Phase advection slip not implemented at this time for linear finite volume");
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addMaterials()
{
  if (_close_mass_transfer_on_area)
    addInterfacialMassTransferRateMaterial();

  // Add the phase fraction variable, for output purposes mostly
  if (!getProblem().hasFunctor(_phase_1_fraction_name, /*thread_id=*/0))
  {
    addParsedFunctorMaterial(prefix() + "phase_1_fraction",
                             _phase_1_fraction_name,
                             "1 - " + _phase_2_fraction_name,
                             {_phase_2_fraction_name},
                             {},
                             true);

    // One of the phase fraction should exist though (either as a variable or set by a
    // NSLiquidFractionAux)
    if (!getProblem().hasFunctor(_phase_2_fraction_name, /*thread_id=*/0))
      paramError("Phase 2 fraction should be defined as a variable or auxiliary variable");
  }
  if (!getProblem().hasFunctor(_phase_2_fraction_name, /*thread_id=*/0))
    addParsedFunctorMaterial(prefix() + "phase_2_fraction",
                             _phase_2_fraction_name,
                             "1 - " + _phase_1_fraction_name,
                             {_phase_1_fraction_name},
                             {},
                             true);

  // Compute mixture properties
  if (!_use_external_mixture_properties)
  {
    auto params = getFactory().getValidParams("WCNSLinearFVMixtureFunctorMaterial");
    assignBlocks(params, _blocks);
    // The specific heat is deliberately absent from this list. This object forms a volume-weighted
    // average, which is correct for the density, the viscosity and the conductivity, but the
    // specific heat has to be mass-weighted for rho_m cp_m T to be the mixture enthalpy density.
    // It is added by addMixtureSpecificHeatMaterial below.
    params.set<std::vector<MooseFunctorName>>("prop_names") = {
        "rho_mixture", "mu_mixture", "k_mixture"};
    // The phase_1 and phase_2 assignments are only local to this object.
    // We use the phase 2 variable to save a functor evaluation as we expect
    // the phase 2 variable to be a nonlinear variable in the phase transport equation
    params.set<std::vector<MooseFunctorName>>("phase_2_names") = {
        _phase_1_density, _phase_1_viscosity, _phase_1_thermal_conductivity};
    params.set<std::vector<MooseFunctorName>>("phase_1_names") = {
        _phase_2_density, _phase_2_viscosity, _phase_2_thermal_conductivity};
    params.set<MooseFunctorName>("phase_1_fraction") = _phase_2_fraction_name;
    if (getParam<bool>("output_all_properties"))
      params.set<std::vector<OutputName>>("outputs") = {"all"};
    params.set<bool>("limit_phase_fraction") = true;
    getProblem().addMaterial(
        "WCNSLinearFVMixtureFunctorMaterial", prefix() + "mixture_material", params);

    addMixtureSpecificHeatMaterial();
  }

  // Compute slip terms as functors, used by the drift flux kernels. The drag model needs them too,
  // since its particle Reynolds number is formed from the slip velocity.
  if (_use_advection_slip || _use_drift_flux || _add_phase_equation ||
      getParam<bool>("use_dispersed_phase_drag_model"))
  {
    mooseAssert(_flow_equations_physics, "We must have coupled to this");
    for (const auto dim : make_range(dimension()))
    {
      const auto object_type = "LinearWCNSFV2PSlipVelocityFunctorMaterial";
      auto params = getFactory().getValidParams(object_type);
      assignBlocks(params, _blocks);
      params.set<MooseFunctorName>("slip_velocity_name") = "vel_slip_" + NS::directions[dim];
      params.set<MooseEnum>("momentum_component") = NS::directions[dim];
      for (const auto j : make_range(dimension()))
        params.set<SolverVariableName>(velocity_components[j]) =
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
      params.set<MooseFunctorName>("drift_velocity_name") = "vel_drift_" + NS::directions[dim];
      params.set<MooseFunctorName>("volumetric_drift_velocity_name") =
          "vel_volumetric_drift_" + NS::directions[dim];
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
      getProblem().addMaterial(object_type, prefix() + "slip_" + NS::directions[dim], params);
    }
  }

  // Coefficient consumed by the phase change term in the energy equation
  if (_fluid_energy_physics && _fluid_energy_physics->hasEnergyEquation() &&
      getParam<bool>("add_phase_change_energy_term"))
    addPhaseChangeCoefficientMaterial();

  // Add a default drag model for a dispersed phase
  if (getParam<bool>("use_dispersed_phase_drag_model"))
  {
    const auto drag_type = "NSFVDispersePhaseDragFunctorMaterial";
    auto params = getFactory().getValidParams(drag_type);
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>("drag_coef_name") = "Darcy_coefficient";
    // The particle Reynolds number is formed from the slip velocity and the continuous phase
    // properties, which is its definition. The material evaluates in AD; the linear finite
    // volume friction kernel takes the raw value through the functor wrapper.
    for (const auto j : make_range(dimension()))
      params.set<MooseFunctorName>(velocity_components[j]) = "vel_slip_" + NS::directions[j];
    params.set<MooseFunctorName>(NS::density) = _phase_1_density;
    params.set<MooseFunctorName>(NS::mu) = _phase_1_viscosity;
    params.set<MooseFunctorName>("particle_diameter") =
        getParam<MooseFunctorName>("particle_diameter");
    if (getParam<bool>("output_all_properties"))
      params.set<std::vector<OutputName>>("outputs") = {"all"};
    getProblem().addMaterial(drag_type, prefix() + "dispersed_drag", params);
  }
}

void
WCNSLinearFVTwoPhaseMixturePhysics::addParsedFunctorMaterial(
    const std::string & object_name,
    const std::string & property_name,
    const std::string & expression,
    const std::vector<std::string> & functor_names,
    const std::vector<std::string> & functor_symbols,
    const bool output)
{
  auto params = getFactory().getValidParams("ParsedFunctorMaterial");
  assignBlocks(params, _blocks);
  params.set<std::string>("expression") = expression;
  params.set<std::vector<std::string>>("functor_names") = functor_names;
  if (!functor_symbols.empty())
    params.set<std::vector<std::string>>("functor_symbols") = functor_symbols;
  params.set<std::string>("property_name") = property_name;
  if (output)
  {
    params.set<std::vector<std::string>>("output_properties") = {property_name};
    params.set<std::vector<OutputName>>("outputs") = {"all"};
  }
  getProblem().addMaterial("ParsedFunctorMaterial", object_name, params);
}
