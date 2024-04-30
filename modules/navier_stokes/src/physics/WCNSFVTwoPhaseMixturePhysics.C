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

  params.addParam<bool>("use_external_mixture_properties",
                        false,
                        "Whether to use the simple NSFVMixtureMaterial or use a more complex model "
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

  params.addParam<bool>("add_drift_flux_momentum_terms",
                        true,
                        "Whether to add the drift flux terms to the momentum equation");

  // Properties of the first phase (can be a liquid or a gas)
  params.renameParam(
      "passive_scalar_names", "phase_fraction_name", "Name of the main phase fraction variable");
  params.addRequiredParam<MooseFunctorName>("first_phase_density_name",
                                            "Name of the density functor for the other phase");
  params.addRequiredParam<MooseFunctorName>("first_phase_viscosity_name",
                                            "Name of the viscosity functor for the other phase");
  params.addRequiredParam<MooseFunctorName>(
      "first_phase_specific_heat_name", "Name of the specific heat functor for the other phase");
  params.addRequiredParam<MooseFunctorName>(
      "first_phase_thermal_conductivity_name",
      "Name of the thermal conductivity functor for the other phase");

  // Properties of the other phase (can be solid, another liquid, or gaseous)
  params.addRequiredParam<MooseFunctorName>(
      "other_phase_fraction_name",
      "Name of the other phase fraction variable, it will be created as a functor material "
      "property if it does not exist already");
  params.addRequiredParam<MooseFunctorName>("other_phase_density_name",
                                            "Name of the density functor for the other phase");
  params.addRequiredParam<MooseFunctorName>("other_phase_viscosity_name",
                                            "Name of the viscosity functor for the other phase");
  params.addRequiredParam<MooseFunctorName>(
      "other_phase_specific_heat_name", "Name of the specific heat functor for the other phase");
  params.addRequiredParam<MooseFunctorName>(
      "other_phase_thermal_conductivity_name",
      "Name of the thermal conductivity functor for the other phase");
  params.addParam<MooseFunctorName>(
      "particle_diameter", 1, "Particle size if using a dispersed phase");

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
  // TODO Rename parameter groups
  params.addParamNamesToGroup("other_phase_density_name other_phase_viscosity_name "
                              "other_phase_specific_heat_name use_external_mixture_properties",
                              "Mixture material properties");

  return params;
}

WCNSFVTwoPhaseMixturePhysics::WCNSFVTwoPhaseMixturePhysics(const InputParameters & parameters)
  : WCNSFVScalarTransportPhysics(parameters),
    _add_phase_equation(_has_scalar_equation),
    _liquid_phase_fraction(_passive_scalar_names[0]),
    _first_phase_density(getParam<MooseFunctorName>("first_phase_density_name")),
    _first_phase_viscosity(getParam<MooseFunctorName>("first_phase_viscosity_name")),
    _first_phase_specific_heat(getParam<MooseFunctorName>("first_phase_specific_heat_name")),
    _first_phase_thermal_conductivity(
        getParam<MooseFunctorName>("first_phase_thermal_conductivity_name")),
    _other_phase_fraction_name(getParam<MooseFunctorName>("other_phase_fraction_name")),
    _other_phase_density(getParam<MooseFunctorName>("other_phase_density_name")),
    _other_phase_viscosity(getParam<MooseFunctorName>("other_phase_viscosity_name")),
    _other_phase_specific_heat(getParam<MooseFunctorName>("other_phase_specific_heat_name")),
    _other_phase_thermal_conductivity(
        getParam<MooseFunctorName>("other_phase_thermal_conductivity_name")),
    _use_external_mixture_properties(getParam<bool>("use_external_mixture_properties")),
    _use_drift_flux(getParam<bool>("add_drift_flux_momentum_terms"))
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

  std::cout << _has_energy_equation << " " << _fluid_energy_physics->hasEnergyEquation()
            << std::endl;

  // Check that the mixture parameters are correctly in use in the other physics
  if (_has_energy_equation)
  {
    if (_fluid_energy_physics->densityName() != "rho_mixture")
      mooseError("Density name should for Physics '",
                 _fluid_energy_physics->name(),
                 "' should be 'rho_mixture'");
    // if (_fluid_energy_physics->dynamicViscosityName() != "mu_mixture")
    //   mooseError("Viscosity name should for Physics '", _fluid_energy_physics->name(), "' should
    //   be 'mu_mixture'");
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
    // if (_flow_equations_physics->dynamicViscosityName() != "mu_mixture")
    //   mooseError("Viscosity name should for Physics ,", _flow_equations_physics->name(), "'
    //   should be 'mu_mixture'");
  }

  if (_verbose)
  {
    if (_flow_equations_physics)
      mooseInfo("Coupled to fluid flow physics " + _flow_equations_physics->name());
    if (_has_energy_equation)
      mooseInfo("Coupled to fluid heat transfer physics " + _fluid_energy_physics->name());
  }
}

void
WCNSFVTwoPhaseMixturePhysics::addFVKernels()
{
  WCNSFVScalarTransportPhysics::addFVKernels();

  if (_add_phase_equation)
    addPhaseInterfaceTerm();

  if (_fluid_energy_physics && _fluid_energy_physics->hasEnergyEquation())
    addPhaseChangeEnergySource();

  if (_flow_equations_physics && _flow_equations_physics->hasFlowEquations() && _use_drift_flux)
    addPhaseDriftFluxTerm();
}

void
WCNSFVTwoPhaseMixturePhysics::addPhaseInterfaceTerm()
{
  auto params = getFactory().getValidParams("NSFVMixturePhaseInterface");
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _liquid_phase_fraction;
  params.set<MooseFunctorName>("phase_coupled") = _other_phase_fraction_name;
  params.set<MaterialPropertyName>("alpha") = getParam<MaterialPropertyName>("alpha");
  getProblem().addFVKernel("NSFVMixturePhaseInterface", prefix() + "phase_interface", params);
}

void
WCNSFVTwoPhaseMixturePhysics::addPhaseChangeEnergySource()
{
  auto params = getFactory().getValidParams("NSFVPhaseChangeSource");
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _fluid_energy_physics->getFluidTemperatureName();
  params.set<MooseFunctorName>("liquid_fraction") = _liquid_phase_fraction;
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
    params.set<MooseFunctorName>("u_slip") = "u_slip";
    if (dimension() >= 2)
      params.set<MooseFunctorName>("v_slip") = "v_slip";
    if (dimension() >= 3)
      params.set<MooseFunctorName>("w_slip") = "w_slip";
    params.set<MooseFunctorName>("rho_d") = _other_phase_density;
    params.set<MooseFunctorName>("fd") = _other_phase_fraction_name;
    params.set<MooseEnum>("component") = components[dim];
    getProblem().addFVKernel(
        "WCNSFV2PMomentumDriftFlux", prefix() + "phase_interface_" + Moose::stringify(dim), params);
  }
}

void
WCNSFVTwoPhaseMixturePhysics::addMaterials()
{
  // Add other phase fraction, for output purposes mostly
  if (!getProblem().hasFunctor(_other_phase_fraction_name, /*thread_id=*/0))
  {
    auto params = getFactory().getValidParams("ADParsedFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<std::string>("expression") = "1 - " + _liquid_phase_fraction;
    params.set<std::vector<std::string>>("functor_names") = {_other_phase_fraction_name};
    params.set<std::string>("property_name") = _other_phase_fraction_name;
    params.set<std::vector<std::string>>("output_properties") = {_other_phase_fraction_name};
    params.set<std::vector<OutputName>>("outputs") = {"all"};
  }

  // Compute mixture properties
  if (!_use_external_mixture_properties)
  {
    auto params = getFactory().getValidParams("NSFVMixtureFunctorMaterial");
    assignBlocks(params, _blocks);
    params.set<std::vector<MooseFunctorName>>("prop_names") = {
        "rho_mixture", "mu_mixture", "cp_mixture", "k_mixture"};
    // The phase_1 and phase_2 assignments are only local to this object.
    params.set<std::vector<MooseFunctorName>>("phase_1_names") = {
        _first_phase_density,
        _first_phase_viscosity,
        _first_phase_specific_heat,
        _first_phase_thermal_conductivity};
    params.set<std::vector<MooseFunctorName>>("phase_2_names") = {
        _other_phase_density,
        _other_phase_viscosity,
        _other_phase_specific_heat,
        _other_phase_thermal_conductivity};
    params.set<MooseFunctorName>("phase_1_fraction") = _liquid_phase_fraction;
    if (getParam<bool>("output_all_properties"))
      params.set<std::vector<OutputName>>("outputs") = {"all"};
    getProblem().addMaterial("NSFVMixtureFunctorMaterial", prefix() + "mixture", params);
  }

  // Compute slip terms as functors, used by the drift flux kernels
  if (_add_phase_equation)
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
        params.set<MooseFunctorName>(vel_components[j]) =
            _flow_equations_physics->getVelocityNames()[j];
      params.set<MooseFunctorName>(NS::density) = _flow_equations_physics->densityName();
      params.set<MooseFunctorName>(NS::mu) = "mu_mixture";
      params.set<MooseFunctorName>("rho_d") = _other_phase_density;
      params.set<RealVectorValue>("gravity") = _flow_equations_physics->gravityVector();
      params.set<MooseFunctorName>("linear_coef_name") =
          _flow_equations_physics->getLinearFrictionCoefName();
      params.set<MooseFunctorName>("particle_diameter") =
          getParam<MooseFunctorName>("particle_diameter");
      if (getParam<bool>("output_all_properties"))
        params.set<std::vector<OutputName>>("outputs") = {"all"};
      getProblem().addMaterial("WCNSFV2PSlipVelocityFunctorMaterial", prefix() + "mixture", params);
    }
  }
}
