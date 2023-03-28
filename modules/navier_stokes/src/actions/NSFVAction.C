//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSFVAction.h"
#include "NS.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseObject.h"
#include "NonlinearSystemBase.h"
#include "RelationshipManager.h"
#include "AuxiliarySystem.h"

registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_variables");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_user_objects");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_ics");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_kernels");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_bcs");
registerMooseAction("NavierStokesApp", NSFVAction, "add_material");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_pps");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_materials");
registerMooseAction("NavierStokesApp", NSFVAction, "navier_stokes_check_copy_nodal_vars");
registerMooseAction("NavierStokesApp", NSFVAction, "navier_stokes_copy_nodal_vars");

InputParameters
NSFVAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("This class allows us to set up Navier-Stokes equations for porous "
                             "medium or clean fluid flows using incompressible or weakly "
                             "compressible approximations with a finite volume discretization.");

  /**
   * General parameters used to set up the simulation.
   */
  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of blocks on which NS equations are defined on");

  MooseEnum comp_type("incompressible weakly-compressible", "incompressible");
  params.addParam<MooseEnum>(
      "compressibility", comp_type, "Compressibility constraint for the Navier-Stokes equations.");

  params.addParam<bool>(
      "porous_medium_treatment", false, "Whether to use porous medium kernels or not.");

  params.addParam<bool>("initialize_variables_from_mesh_file",
                        false,
                        "Determines if the variables that are added by the action are initialized "
                        "from the mesh file (only for Exodus format)");
  params.addParam<std::string>(
      "initial_from_file_timestep",
      "LATEST",
      "Gives the timestep (or \"LATEST\") for which to read a solution from a file "
      "for a given variable. (Default: LATEST)");

  MooseEnum turbulence_type("mixing-length none", "none");
  params.addParam<MooseEnum>(
      "turbulence_handling",
      turbulence_type,
      "The way additional diffusivities are determined in the turbulent regime.");

  params.addParam<bool>("add_flow_equations", true, "True to add mass and momentum equations");
  params.addParam<bool>("add_energy_equation", false, "True to add energy equation");
  params.addParam<bool>("add_scalar_equation", false, "True to add advected scalar(s) equation");

  params.addParamNamesToGroup("compressibility porous_medium_treatment turbulence_handling "
                              "add_flow_equations add_energy_equation add_scalar_equation ",
                              "General control");

  params.addParam<std::vector<std::string>>(
      "velocity_variable",
      "If supplied, the system checks for available velocity variables. "
      "Otherwise, they are created within the action.");

  params.addParam<NonlinearVariableName>("pressure_variable",
                                         "If supplied, the system checks for available pressure "
                                         "variable. Otherwise, it is created within the action.");

  params.addParam<NonlinearVariableName>(
      "fluid_temperature_variable",
      "If supplied, the system checks for available fluid "
      "temperature variable. Otherwise, it is created within the action.");

  params.addParamNamesToGroup("velocity_variable pressure_variable fluid_temperature_variable",
                              "External variable");

  /**
   * Parameters influencing the porous medium treatment.
   */

  params.addParam<MooseFunctorName>(
      "porosity", NS::porosity, "The name of the auxiliary variable for the porosity field.");

  params.addParam<unsigned short>(
      "porosity_smoothing_layers",
      "The number of interpolation-reconstruction operations to perform on the porosity.");

  MooseEnum porosity_interface_pressure_treatment("automatic bernoulli", "automatic");
  params.addParam<MooseEnum>("porosity_interface_pressure_treatment",
                             porosity_interface_pressure_treatment,
                             "How to treat pressure at a porosity interface");

  params.addParam<bool>("use_friction_correction",
                        "If friction correction should be applied in the momentum equation.");

  params.addParam<Real>(
      "consistent_scaling",
      "Scaling parameter for the friction correction in the momentum equation (if requested).");

  params.addParamNamesToGroup("porosity porosity_smoothing_layers use_friction_correction "
                              "consistent_scaling porosity_interface_pressure_treatment",
                              "Porous medium treatment");

  /**
   * Parameters used to define the boundaries of the domain.
   */

  params.addParam<std::vector<BoundaryName>>(
      "inlet_boundaries", std::vector<BoundaryName>(), "Names of inlet boundaries");
  params.addParam<std::vector<BoundaryName>>(
      "outlet_boundaries", std::vector<BoundaryName>(), "Names of outlet boundaries");
  params.addParam<std::vector<BoundaryName>>(
      "wall_boundaries", std::vector<BoundaryName>(), "Names of wall boundaries");

  /**
   * Parameters used to define the handling of the momentum-mass equations.
   */
  std::vector<FunctionName> default_initial_velocity = {"1e-15", "1e-15", "1e-15"};
  params.addParam<std::vector<FunctionName>>("initial_velocity",
                                             default_initial_velocity,
                                             "The initial velocity, assumed constant everywhere");

  params.addParam<FunctionName>(
      "initial_pressure", "1e5", "The initial pressure, assumed constant everywhere");

  params.addParam<MooseFunctorName>(
      "dynamic_viscosity", NS::mu, "The name of the dynamic viscosity");

  params.addParam<MooseFunctorName>("density", NS::density, "The name of the density");

  params.addParam<RealVectorValue>(
      "gravity", RealVectorValue(0, 0, 0), "The gravitational acceleration vector.");

  MultiMooseEnum mom_inlet_types("fixed-velocity flux-velocity flux-mass fixed-pressure");
  params.addParam<MultiMooseEnum>("momentum_inlet_types",
                                  mom_inlet_types,
                                  "Types of inlet boundaries for the momentum equation.");

  params.addParam<std::vector<std::vector<FunctionName>>>(
      "momentum_inlet_function",
      std::vector<std::vector<FunctionName>>(),
      "Functions for inlet boundary velocities or pressures (for fixed-pressure option). Provide a "
      "double vector where the leading dimension corresponds to the number of fixed-velocity and "
      "fixed-pressure entries in momentum_inlet_types and the second index runs either over "
      "dimensions for fixed-velocity boundaries or is a single function name for pressure inlets.");
  params.addParam<std::vector<PostprocessorName>>(
      "flux_inlet_pps",
      std::vector<PostprocessorName>(),
      "The name of the postprocessors which compute the mass flow/ velocity magnitude. "
      "Mainly used for coupling between different applications.");
  params.addParam<std::vector<Point>>(
      "flux_inlet_directions",
      std::vector<Point>(),
      "The directions which can be used to define the orientation of the flux with respect to the "
      "mesh. This can be used to define a flux which is incoming with an angle or to adjust the "
      "flux direction with respect to the normal. If the inlet surface is defined on an internal "
      "face, this is necessary to ensure the arbitrary orientation of the normal does not result "
      "in non-physical results.");

  MultiMooseEnum mom_outlet_types("fixed-pressure zero-gradient fixed-pressure-zero-gradient");
  params.addParam<MultiMooseEnum>("momentum_outlet_types",
                                  mom_outlet_types,
                                  "Types of outlet boundaries for the momentum equation");
  params.addParam<std::vector<FunctionName>>("pressure_function",
                                             std::vector<FunctionName>(),
                                             "Functions for boundary pressures at outlets.");

  MultiMooseEnum mom_wall_types("symmetry noslip slip wallfunction", "noslip");
  params.addParam<MultiMooseEnum>(
      "momentum_wall_types", mom_wall_types, "Types of wall boundaries for the momentum equation");

  params.addParam<bool>(
      "pin_pressure", false, "Switch to enable pressure shifting for incompressible simulations.");

  MooseEnum s_type("average point-value", "average");
  params.addParam<MooseEnum>(
      "pinned_pressure_type",
      s_type,
      "Types for shifting (pinning) the pressure in case of incompressible simulations.");

  params.addParam<Point>(
      "pinned_pressure_point",
      Point(),
      "The XYZ coordinates where pressure needs to be pinned for incompressible simulations.");

  params.addParam<PostprocessorName>(
      "pinned_pressure_value",
      "1e5",
      "The value used for pinning the pressure (point value/domain average).");

  params.addParam<bool>("boussinesq_approximation", false, "True to have Boussinesq approximation");

  params.addRangeCheckedParam<Real>(
      "ref_temperature",
      273.15,
      "ref_temperature > 0.0",
      "Value for reference temperature in case of Boussinesq approximation");
  params.addParam<MooseFunctorName>(
      "thermal_expansion",
      NS::alpha,
      "The name of the thermal expansion coefficient in the Boussinesq approximation");

  params.addParamNamesToGroup(
      "pin_pressure pinned_pressure_type pinned_pressure_point pinned_pressure_value "
      "ref_temperature boussinesq_approximation gravity",
      "Momentum equation");

  /**
   * Equations used to set up the energy equation/enthalpy equation if it is required.
   */

  params.addParam<FunctionName>(
      "initial_temperature", "300", "The initial temperature, assumed constant everywhere");

  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "thermal_conductivity_blocks",
      "The blocks where the user wants define different thermal conductivities.");

  params.addParam<std::vector<MooseFunctorName>>(
      "thermal_conductivity",
      std::vector<MooseFunctorName>({NS::k}),
      "The name of the fluid thermal conductivity for each block");

  params.addParam<MooseFunctorName>("specific_heat", NS::cp, "The name of the specific heat");

  MultiMooseEnum en_inlet_types("fixed-temperature flux-mass flux-velocity heatflux");
  params.addParam<MultiMooseEnum>("energy_inlet_types",
                                  en_inlet_types,
                                  "Types for the inlet boundaries for the energy equation.");

  params.addParam<std::vector<std::string>>(
      "energy_inlet_function",
      std::vector<std::string>(),
      "Functions for fixed-value boundaries in the energy equation.");

  MultiMooseEnum en_wall_types("fixed-temperature heatflux", "heatflux");
  params.addParam<MultiMooseEnum>(
      "energy_wall_types", en_wall_types, "Types for the wall boundaries for the energy equation.");

  params.addParam<std::vector<FunctionName>>(
      "energy_wall_function",
      std::vector<FunctionName>(),
      "Functions for Dirichlet/Neumann boundaries in the energy equation.");

  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "ambient_convection_blocks",
      std::vector<std::vector<SubdomainName>>(),
      "The blocks where the ambient convection is present.");

  params.addParam<std::vector<MooseFunctorName>>(
      "ambient_convection_alpha",
      std::vector<MooseFunctorName>(),
      "The heat exchange coefficients for each block in 'ambient_convection_blocks'.");

  params.addParam<std::vector<MooseFunctorName>>(
      "ambient_temperature",
      std::vector<MooseFunctorName>(),
      "The ambient temperature for each block in 'ambient_convection_blocks'.");

  params.addParam<MooseFunctorName>(
      "external_heat_source",
      "The name of a functor which contains the external heat source for the energy equation.");
  params.addParam<Real>(
      "external_heat_source_coeff", 1.0, "Multiplier for the coupled heat source term.");
  params.addParam<bool>("use_external_enthalpy_material",
                        false,
                        "To indicate if the enthalpy material is set up outside of the action.");

  params.addParamNamesToGroup("ambient_convection_alpha ambient_convection_blocks "
                              "ambient_temperature external_heat_source external_heat_source_coeff "
                              "use_external_enthalpy_material",
                              "Energy equation");

  /**
   * Parameters controlling the friction terms in case of porous medium simulations.
   */

  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "friction_blocks",
      "The blocks where the friction factors are applied to emulate flow resistances.");

  params.addParam<std::vector<std::vector<std::string>>>(
      "friction_types", "The types of friction forces for every block in 'friction_blocks'.");

  params.addParam<std::vector<std::vector<std::string>>>(
      "friction_coeffs",
      "The friction coefficients for every item in 'friction_types'. Note that if "
      "'porous_medium_treatment' is enabled, the coefficients already contain a velocity "
      "multiplier but they are not multiplied with density yet!");

  params.addParamNamesToGroup("friction_blocks friction_types friction_coeffs", "Friction control");

  /**
   * Parameters describing the handling of advected scalar fields
   */

  params.addParam<std::vector<NonlinearVariableName>>(
      "passive_scalar_names",
      std::vector<NonlinearVariableName>(),
      "Vector containing the names of the advected scalar variables.");

  params.addParam<std::vector<FunctionName>>("initial_scalar_variables",
                                             "Initial values of the passive scalar variables.");

  params.addParam<std::vector<MooseFunctorName>>(
      "passive_scalar_diffusivity",
      std::vector<MooseFunctorName>(),
      "Functor names for the diffusivities used for the passive scalar fields.");

  params.addParam<std::vector<Real>>("passive_scalar_schmidt_number",
                                     std::vector<Real>(),
                                     "Schmidt numbers used for the passive scalar fields.");

  params.addParam<std::vector<MooseFunctorName>>(
      "passive_scalar_source",
      std::vector<MooseFunctorName>(),
      "Functor names for the sources used for the passive scalar fields.");

  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "passive_scalar_coupled_source",
      std::vector<std::vector<MooseFunctorName>>(),
      "Coupled variable names for the sources used for the passive scalar fields. If multiple "
      "sources for each equation are specified, major (outer) ordering by equation.");

  params.addParam<std::vector<std::vector<Real>>>(
      "passive_scalar_coupled_source_coeff",
      std::vector<std::vector<Real>>(),
      "Coupled variable multipliers for the sources used for the passive scalar fields. If multiple"
      " sources for each equation are specified, major (outer) ordering by equation.");

  MultiMooseEnum ps_inlet_types("fixed-value flux-mass flux-velocity", "fixed-value");
  params.addParam<MultiMooseEnum>(
      "passive_scalar_inlet_types",
      ps_inlet_types,
      "Types for the inlet boundaries for the passive scalar equation.");

  params.addParam<std::vector<std::vector<std::string>>>(
      "passive_scalar_inlet_function",
      std::vector<std::vector<std::string>>(),
      "Functions for inlet boundaries in the passive scalar equations.");

  params.addParamNamesToGroup("passive_scalar_names passive_scalar_diffusivity "
                              "passive_scalar_schmidt_number passive_scalar_source "
                              "passive_scalar_coupled_source passive_scalar_coupled_source_coeff",
                              "Passive scalar control");

  /**
   * Parameters allowing the control over numerical schemes for different terms in the
   * Navier-Stokes + energy equations.
   */

  MooseEnum adv_interpol_types("average upwind skewness-corrected min_mod vanLeer", "average");
  params.addParam<MooseEnum>("mass_advection_interpolation",
                             adv_interpol_types,
                             "The numerical scheme to use for interpolating density, "
                             "as an advected quantity, to the face.");
  params.addParam<MooseEnum>("momentum_advection_interpolation",
                             adv_interpol_types,
                             "The numerical scheme to use for interpolating momentum/velocity, "
                             "as an advected quantity, to the face.");
  params.addParam<MooseEnum>("energy_advection_interpolation",
                             adv_interpol_types,
                             "The numerical scheme to use for interpolating energy/temperature, "
                             "as an advected quantity, to the face.");
  params.addParam<MooseEnum>("passive_scalar_advection_interpolation",
                             adv_interpol_types,
                             "The numerical scheme to use for interpolating passive scalar field, "
                             "as an advected quantity, to the face.");

  MooseEnum face_interpol_types("average skewness-corrected", "average");
  params.addParam<MooseEnum>("pressure_face_interpolation",
                             face_interpol_types,
                             "The numerical scheme to interpolate the pressure to the "
                             "face (separate from the advected quantity interpolation).");
  params.addParam<MooseEnum>("momentum_face_interpolation",
                             face_interpol_types,
                             "The numerical scheme to interpolate the velocity/momentum to the "
                             "face (separate from the advected quantity interpolation).");
  params.addParam<MooseEnum>("energy_face_interpolation",
                             face_interpol_types,
                             "The numerical scheme to interpolate the temperature/energy to the "
                             "face (separate from the advected quantity interpolation).");
  params.addParam<MooseEnum>(
      "passive_scalar_face_interpolation",
      face_interpol_types,
      "The numerical scheme to interpolate the passive scalar field variables to the "
      "face (separate from the advected quantity interpolation).");

  MooseEnum velocity_interpolation("average rc", "rc");
  params.addParam<MooseEnum>(
      "velocity_interpolation",
      velocity_interpolation,
      "The interpolation to use for the velocity. Options are "
      "'average' and 'rc' which stands for Rhie-Chow. The default is Rhie-Chow.");

  params.addParam<bool>(
      "pressure_two_term_bc_expansion",
      true,
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the pressure.");
  params.addParam<bool>(
      "momentum_two_term_bc_expansion",
      true,
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the velocity/momentum.");
  params.addParam<bool>(
      "energy_two_term_bc_expansion",
      true,
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the temperature/energy.");
  params.addParam<bool>(
      "passive_scalar_two_term_bc_expansion",
      true,
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the advected passive scalar field.");
  params.addParam<bool>(
      "mixing_length_two_term_bc_expansion",
      true,
      "If a two-term Taylor expansion is needed for the determination of the boundary values"
      "of the mixing length field.");

  params.addRangeCheckedParam<Real>(
      "mass_scaling",
      1.0,
      "mass_scaling > 0.0",
      "The scaling factor for the mass variables (for incompressible simulation "
      "this is pressure scaling).");
  params.addRangeCheckedParam<Real>("momentum_scaling",
                                    1.0,
                                    "momentum_scaling > 0.0",
                                    "The scaling factor for the momentum variables.");
  params.addRangeCheckedParam<Real>(
      "energy_scaling", 1.0, "energy_scaling > 0.0", "The scaling factor for the energy variable.");
  params.addRangeCheckedParam<Real>("passive_scalar_scaling",
                                    1.0,
                                    "passive_scalar_scaling > 0.0",
                                    "The scaling factor for the passive scalar field variables.");

  params.addParamNamesToGroup(
      "momentum_advection_interpolation energy_advection_interpolation "
      "passive_scalar_advection_interpolation mass_advection_interpolation "
      "momentum_face_interpolation energy_face_interpolation passive_scalar_face_interpolation "
      "pressure_face_interpolation momentum_two_term_bc_expansion "
      "energy_two_term_bc_expansion passive_scalar_two_term_bc_expansion "
      "mixing_length_two_term_bc_expansion pressure_two_term_bc_expansion "
      "velocity_interpolation",
      "Numerical scheme");

  params.addParamNamesToGroup("momentum_scaling energy_scaling mass_scaling passive_scalar_scaling",
                              "Scaling");

  /**
   * Parameter controlling the turbulence handling used for the equations.
   */
  params.addParam<std::vector<BoundaryName>>(
      "mixing_length_walls",
      std::vector<BoundaryName>(),
      "Walls where the mixing length model should be utilized.");

  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  params.addParam<ExecFlagEnum>("mixing_length_aux_execute_on",
                                exec_enum,
                                "When the mixing length aux kernels should be executed.");

  params.addRangeCheckedParam<Real>("von_karman_const",
                                    0.41,
                                    "von_karman_const > 0.0",
                                    "Von Karman parameter for the mixing length model");
  params.addRangeCheckedParam<Real>(
      "von_karman_const_0", 0.09, "von_karman_const_0 > 0.0", "'Escudier' model parameter");
  params.addRangeCheckedParam<Real>(
      "mixing_length_delta",
      1.0,
      "mixing_length_delta > 0.0",
      "Tunable parameter related to the thickness of the boundary layer."
      "When it is not specified, Prandtl's original unbounded wall distance mixing length model is"
      "retrieved.");
  params.addRangeCheckedParam<Real>("turbulent_prandtl",
                                    1,
                                    "turbulent_prandtl > 0",
                                    "Turbulent Prandtl number for energy turbulent diffusion");

  params.addParamNamesToGroup("mixing_length_walls mixing_length_aux_execute_on von_karman_const "
                              "von_karman_const_0 mixing_length_delta turbulent_prandtl",
                              "Turbulence");

  // Create input parameter groups

  params.addParamNamesToGroup("dynamic_viscosity density thermal_expansion "
                              "thermal_conductivity_blocks thermal_conductivity specific_heat",
                              "Material property");

  params.addParamNamesToGroup(
      "inlet_boundaries momentum_inlet_types momentum_inlet_function energy_inlet_types "
      "energy_inlet_function wall_boundaries momentum_wall_types energy_wall_types "
      "energy_wall_function outlet_boundaries momentum_outlet_types pressure_function "
      "passive_scalar_inlet_types passive_scalar_inlet_function flux_inlet_pps "
      "flux_inlet_directions",
      "Boundary condition");

  params.addParamNamesToGroup(
      "initial_pressure initial_velocity initial_temperature initial_scalar_variables "
      "initialize_variables_from_mesh_file initial_from_file_timestep",
      "Initial condition");

  return params;
}

NSFVAction::NSFVAction(const InputParameters & parameters)
  : Action(parameters),
    _blocks(getParam<std::vector<SubdomainName>>("block")),
    _compressibility(getParam<MooseEnum>("compressibility")),
    _has_flow_equations(getParam<bool>("add_flow_equations")),
    _has_energy_equation(getParam<bool>("add_energy_equation")),
    _has_scalar_equation(getParam<bool>("add_scalar_equation")),
    _boussinesq_approximation(getParam<bool>("boussinesq_approximation")),
    _turbulence_handling(getParam<MooseEnum>("turbulence_handling")),
    _porous_medium_treatment(getParam<bool>("porous_medium_treatment")),
    _porosity_name(getParam<MooseFunctorName>("porosity")),
    _flow_porosity_functor_name(isParamValid("porosity_smoothing_layers") &&
                                        getParam<unsigned short>("porosity_smoothing_layers")
                                    ? NS::smoothed_porosity
                                    : _porosity_name),
    _use_friction_correction(isParamValid("use_friction_correction")
                                 ? getParam<bool>("use_friction_correction")
                                 : false),
    _velocity_name(
        isParamValid("velocity_variable")
            ? getParam<std::vector<std::string>>("velocity_variable")
            : (_porous_medium_treatment
                   ? std::vector<std::string>(NS::superficial_velocity_vector,
                                              NS::superficial_velocity_vector + 3)
                   : std::vector<std::string>(NS::velocity_vector, NS::velocity_vector + 3))),
    _pressure_name(isParamValid("pressure_variable")
                       ? getParam<NonlinearVariableName>("pressure_variable")
                       : NS::pressure),
    _fluid_temperature_name(isParamValid("fluid_temperature_variable")
                                ? getParam<NonlinearVariableName>("fluid_temperature_variable")
                                : NS::T_fluid),
    _inlet_boundaries(getParam<std::vector<BoundaryName>>("inlet_boundaries")),
    _outlet_boundaries(getParam<std::vector<BoundaryName>>("outlet_boundaries")),
    _wall_boundaries(getParam<std::vector<BoundaryName>>("wall_boundaries")),
    _momentum_inlet_types(getParam<MultiMooseEnum>("momentum_inlet_types")),
    _flux_inlet_pps(getParam<std::vector<PostprocessorName>>("flux_inlet_pps")),
    _flux_inlet_directions(getParam<std::vector<Point>>("flux_inlet_directions")),
    _momentum_inlet_function(
        getParam<std::vector<std::vector<FunctionName>>>("momentum_inlet_function")),
    _momentum_outlet_types(getParam<MultiMooseEnum>("momentum_outlet_types")),
    _momentum_wall_types(getParam<MultiMooseEnum>("momentum_wall_types")),
    _energy_inlet_types(getParam<MultiMooseEnum>("energy_inlet_types")),
    _energy_inlet_function(getParam<std::vector<std::string>>("energy_inlet_function")),
    _energy_wall_types(getParam<MultiMooseEnum>("energy_wall_types")),
    _energy_wall_function(getParam<std::vector<FunctionName>>("energy_wall_function")),
    _pressure_function(getParam<std::vector<FunctionName>>("pressure_function")),
    _ambient_convection_blocks(
        getParam<std::vector<std::vector<SubdomainName>>>("ambient_convection_blocks")),
    _ambient_convection_alpha(getParam<std::vector<MooseFunctorName>>("ambient_convection_alpha")),
    _ambient_temperature(getParam<std::vector<MooseFunctorName>>("ambient_temperature")),
    _friction_blocks(isParamValid("friction_blocks")
                         ? getParam<std::vector<std::vector<SubdomainName>>>("friction_blocks")
                         : std::vector<std::vector<SubdomainName>>()),
    _friction_types(isParamValid("friction_types")
                        ? getParam<std::vector<std::vector<std::string>>>("friction_types")
                        : std::vector<std::vector<std::string>>()),
    _friction_coeffs(isParamValid("friction_coeffs")
                         ? getParam<std::vector<std::vector<std::string>>>("friction_coeffs")
                         : std::vector<std::vector<std::string>>()),
    _density_name(getParam<MooseFunctorName>("density")),
    _dynamic_viscosity_name(getParam<MooseFunctorName>("dynamic_viscosity")),
    _specific_heat_name(getParam<MooseFunctorName>("specific_heat")),
    _thermal_conductivity_blocks(
        isParamValid("thermal_conductivity_blocks")
            ? getParam<std::vector<std::vector<SubdomainName>>>("thermal_conductivity_blocks")
            : std::vector<std::vector<SubdomainName>>()),
    _thermal_conductivity_name(getParam<std::vector<MooseFunctorName>>("thermal_conductivity")),
    _thermal_expansion_name(getParam<MooseFunctorName>("thermal_expansion")),
    _passive_scalar_names(getParam<std::vector<NonlinearVariableName>>("passive_scalar_names")),
    _passive_scalar_diffusivity(
        getParam<std::vector<MooseFunctorName>>("passive_scalar_diffusivity")),
    _passive_scalar_schmidt_number(getParam<std::vector<Real>>("passive_scalar_schmidt_number")),
    _passive_scalar_source(getParam<std::vector<MooseFunctorName>>("passive_scalar_source")),
    _passive_scalar_coupled_source(
        getParam<std::vector<std::vector<MooseFunctorName>>>("passive_scalar_coupled_source")),
    _passive_scalar_coupled_source_coeff(
        getParam<std::vector<std::vector<Real>>>("passive_scalar_coupled_source_coeff")),
    _passive_scalar_inlet_types(getParam<MultiMooseEnum>("passive_scalar_inlet_types")),
    _passive_scalar_inlet_function(
        getParam<std::vector<std::vector<std::string>>>("passive_scalar_inlet_function")),
    _mass_advection_interpolation(getParam<MooseEnum>("mass_advection_interpolation")),
    _momentum_advection_interpolation(getParam<MooseEnum>("momentum_advection_interpolation")),
    _energy_advection_interpolation(getParam<MooseEnum>("energy_advection_interpolation")),
    _passive_scalar_advection_interpolation(
        getParam<MooseEnum>("passive_scalar_advection_interpolation")),
    _pressure_face_interpolation(getParam<MooseEnum>("pressure_face_interpolation")),
    _momentum_face_interpolation(getParam<MooseEnum>("momentum_face_interpolation")),
    _energy_face_interpolation(getParam<MooseEnum>("energy_face_interpolation")),
    _passive_scalar_face_interpolation(getParam<MooseEnum>("passive_scalar_face_interpolation")),
    _velocity_interpolation(getParam<MooseEnum>("velocity_interpolation")),
    _pressure_two_term_bc_expansion(getParam<bool>("pressure_two_term_bc_expansion")),
    _momentum_two_term_bc_expansion(getParam<bool>("momentum_two_term_bc_expansion")),
    _energy_two_term_bc_expansion(getParam<bool>("energy_two_term_bc_expansion")),
    _passive_scalar_two_term_bc_expansion(getParam<bool>("passive_scalar_two_term_bc_expansion")),
    _mass_scaling(getParam<Real>("mass_scaling")),
    _momentum_scaling(getParam<Real>("momentum_scaling")),
    _energy_scaling(getParam<Real>("energy_scaling")),
    _passive_scalar_scaling(getParam<Real>("passive_scalar_scaling")),
    _create_velocity(!isParamValid("velocity_variable")),
    _create_pressure(!isParamValid("pressure_variable")),
    _create_fluid_temperature(!isParamValid("fluid_temperature_variable")),
    _use_external_enthalpy_material(getParam<bool>("use_external_enthalpy_material"))
{
  // Running the general checks, the rest are run after we already know some
  // geometry-related parameters.
  checkGeneralControlErrors();
}

void
NSFVAction::act()
{
  if (_current_task == "add_navier_stokes_variables")
  {
    // We process parameters necesary to handle block-restriction
    processMesh();

    // We check if we need to create variables
    processVariables();

    // The condition is used because this action will incorporate compressible simulations in the
    // future. The same applied to the other conditions below.
    if (_compressibility == "weakly-compressible" || _compressibility == "incompressible")
      addINSVariables();
  }

  if (_current_task == "add_navier_stokes_user_objects")
  {
    if (_compressibility == "incompressible" || _compressibility == "weakly-compressible")
      addRhieChowUserObjects();
  }

  if (_current_task == "add_navier_stokes_ics")
  {
    // Check initial condition related user input errors
    checkICParameterErrors();

    if (_compressibility == "incompressible" || _compressibility == "weakly-compressible")
      addINSInitialConditions();
  }

  if (_current_task == "add_navier_stokes_kernels")
  {
    // Check if the user made mistakes in the definition of friction parameters
    checkFrictionParameterErrors();

    // Check if the user made mistakes in the definition of ambient convection parameters
    checkAmbientConvectionParameterErrors();

    // Check if the user made mistakes in the definition of scalar kernel parameters
    checkPassiveScalarParameterErrors();

    if (_has_flow_equations)
    {
      if (_compressibility == "incompressible")
      {
        if (_problem->isTransient())
        {
          addINSMomentumTimeKernels();
          if (_has_energy_equation)
            addINSEnergyTimeKernels();
        }
        if (_boussinesq_approximation)
          addINSMomentumBoussinesqKernels();
      }
      else
      {
        if (_problem->isTransient())
        {
          addWCNSMassTimeKernels();
          addWCNSMomentumTimeKernels();
          if (_has_energy_equation)
            addWCNSEnergyTimeKernels();
        }
      }
    }

    if (_compressibility == "incompressible" || _compressibility == "weakly-compressible")
    {
      // If the material properties are not constant, we can use the same kernels
      // for weakly-compressible simulations.
      if (_has_flow_equations)
      {
        addINSMassKernels();
        addINSMomentumAdvectionKernels();
        addINSMomentumViscousDissipationKernels();
        addINSMomentumPressureKernels();
        addINSMomentumGravityKernels();
        if (_friction_types.size())
          addINSMomentumFrictionKernels();

        if (_turbulence_handling == "mixing-length")
          addINSMomentumMixingLengthKernels();
      }

      if (_has_energy_equation)
      {
        addINSEnergyAdvectionKernels();
        addINSEnergyHeatConductionKernels();
        if (_ambient_temperature.size())
          addINSEnergyAmbientConvection();
        if (isParamValid("external_heat_source"))
          addINSEnergyExternalHeatSource();

        if (_turbulence_handling == "mixing-length")
          addWCNSEnergyMixingLengthKernels();
      }
      if (_has_scalar_equation)
      {
        if (_problem->isTransient())
          addScalarTimeKernels();

        addScalarAdvectionKernels();
        if (_passive_scalar_diffusivity.size())
          addScalarDiffusionKernels();
        if (_turbulence_handling == "mixing-length")
          addScalarMixingLengthKernels();
        if (_passive_scalar_source.size())
          addScalarSourceKernels();
        if (_passive_scalar_coupled_source.size())
          addScalarCoupledSourceKernels();
      }
    }
  }

  if (_current_task == "add_navier_stokes_bcs")
  {
    if (_compressibility == "incompressible" || _compressibility == "weakly-compressible")
    {
      addINSInletBC();
      addINSOutletBC();
      if (_has_flow_equations)
        addINSWallBC();
      if (_has_energy_equation)
      {
        addINSEnergyInletBC();
        addINSEnergyWallBC();
      }

      if (_has_scalar_equation)
        addScalarInletBC();
    }
  }

  if (_current_task == "add_navier_stokes_materials")
  {
    if (_compressibility == "incompressible" || _compressibility == "weakly-compressible")
    {
      if (_has_energy_equation && !_use_external_enthalpy_material)
        addEnthalpyMaterial();
      if (_porous_medium_treatment)
        addPorousMediumSpeedMaterial();
      if (_turbulence_handling == "mixing-length")
        addMixingLengthMaterial();
    }
  }

  if (_current_task == "add_navier_stokes_pps")
  {
    // Check if the user defined the boundary conditions in a sensible way
    checkBoundaryParameterErrors();

    addBoundaryPostprocessors();
  }

  if (getParam<bool>("initialize_variables_from_mesh_file"))
  {
    if (_current_task == "navier_stokes_check_copy_nodal_vars")
      _app.setExodusFileRestart(true);
    else if (_current_task == "navier_stokes_copy_nodal_vars")
    {
      SystemBase & system = _problem->getNonlinearSystemBase();

      if (_create_pressure)
        system.addVariableToCopy(
            _pressure_name, _pressure_name, getParam<std::string>("initial_from_file_timestep"));

      if (_create_velocity)
        for (unsigned int d = 0; d < _dim; ++d)
          system.addVariableToCopy(_velocity_name[d],
                                   _velocity_name[d],
                                   getParam<std::string>("initial_from_file_timestep"));

      if (getParam<bool>("pin_pressure"))
        system.addVariableToCopy(
            "lambda", "lambda", getParam<std::string>("initial_from_file_timestep"));

      if (_turbulence_handling == "mixing-length")
        _problem->getAuxiliarySystem().addVariableToCopy(
            NS::mixing_length,
            NS::mixing_length,
            getParam<std::string>("initial_from_file_timestep"));

      if (_has_energy_equation && _create_fluid_temperature)
        system.addVariableToCopy(_fluid_temperature_name,
                                 _fluid_temperature_name,
                                 getParam<std::string>("initial_from_file_timestep"));

      if (_has_scalar_equation)
        for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
        {
          bool create_me = true;
          if (_create_scalar_variable.size())
            if (!_create_scalar_variable[name_i])
              create_me = false;
          if (create_me)
            system.addVariableToCopy(_passive_scalar_names[name_i],
                                     _passive_scalar_names[name_i],
                                     getParam<std::string>("initial_from_file_timestep"));
        }
    }
  }
}

void
NSFVAction::addINSVariables()
{
  // Add velocity variable
  if (_create_velocity)
  {
    std::string variable_type = "INSFVVelocityVariable";
    if (_porous_medium_treatment)
      variable_type = "PINSFVSuperficialVelocityVariable";

    auto params = _factory.getValidParams(variable_type);
    assignBlocks(params, _blocks);
    params.set<std::vector<Real>>("scaling") = {_momentum_scaling};
    params.set<MooseEnum>("face_interp_method") = _momentum_face_interpolation;
    params.set<bool>("two_term_boundary_expansion") = _momentum_two_term_bc_expansion;

    for (unsigned int d = 0; d < _dim; ++d)
      _problem->addVariable(variable_type, _velocity_name[d], params);
  }

  // Add pressure variable
  if (_create_pressure)
  {
    const bool using_pinsfv_pressure_var =
        _porous_medium_treatment &&
        getParam<MooseEnum>("porosity_interface_pressure_treatment") != "automatic";
    const auto pressure_type =
        using_pinsfv_pressure_var ? "BernoulliPressureVariable" : "INSFVPressureVariable";
    auto params = _factory.getValidParams(pressure_type);
    assignBlocks(params, _blocks);
    params.set<std::vector<Real>>("scaling") = {_mass_scaling};
    params.set<MooseEnum>("face_interp_method") = _pressure_face_interpolation;
    params.set<bool>("two_term_boundary_expansion") = _pressure_two_term_bc_expansion;
    if (using_pinsfv_pressure_var)
    {
      params.set<MooseFunctorName>("u") = _velocity_name[0];
      if (_dim >= 2)
        params.set<MooseFunctorName>("v") = _velocity_name[1];
      if (_dim == 3)
        params.set<MooseFunctorName>("w") = _velocity_name[2];
      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
    }

    _problem->addVariable(pressure_type, _pressure_name, params);
  }

  // Add lagrange multiplier for pinning pressure, if needed
  if (getParam<bool>("pin_pressure"))
  {
    auto lm_params = _factory.getValidParams("MooseVariableScalar");
    lm_params.set<MooseEnum>("family") = "scalar";
    lm_params.set<MooseEnum>("order") = "first";

    _problem->addVariable("MooseVariableScalar", "lambda", lm_params);
  }

  // Add turbulence-related variables
  if (_turbulence_handling == "mixing-length")
  {
    auto params = _factory.getValidParams("MooseVariableFVReal");
    assignBlocks(params, _blocks);
    params.set<bool>("two_term_boundary_expansion") =
        getParam<bool>("mixing_length_two_term_bc_expansion");

    _problem->addAuxVariable("MooseVariableFVReal", NS::mixing_length, params);
  }

  // Add energy variables if needed
  if (_has_energy_equation)
  {
    if (_create_fluid_temperature)
    {
      auto params = _factory.getValidParams("INSFVEnergyVariable");
      assignBlocks(params, _blocks);
      params.set<std::vector<Real>>("scaling") = {_energy_scaling};
      params.set<MooseEnum>("face_interp_method") = _energy_face_interpolation;
      params.set<bool>("two_term_boundary_expansion") = _energy_two_term_bc_expansion;

      _problem->addVariable("INSFVEnergyVariable", _fluid_temperature_name, params);
    }
  }

  // Add passive scalar variables is needed
  if (_has_scalar_equation)
  {
    auto params = _factory.getValidParams("INSFVScalarFieldVariable");
    assignBlocks(params, _blocks);
    params.set<std::vector<Real>>("scaling") = {_passive_scalar_scaling};
    params.set<MooseEnum>("face_interp_method") = _passive_scalar_face_interpolation;
    params.set<bool>("two_term_boundary_expansion") = _passive_scalar_two_term_bc_expansion;

    for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
    {
      bool create_me = true;
      if (_create_scalar_variable.size())
        if (!_create_scalar_variable[name_i])
          create_me = false;

      if (create_me)
        _problem->addVariable("INSFVScalarFieldVariable", _passive_scalar_names[name_i], params);
    }
  }
}

void
NSFVAction::addRhieChowUserObjects()
{
  const std::string u_names[3] = {"u", "v", "w"};
  if (_porous_medium_treatment)
  {
    auto params = _factory.getValidParams("PINSFVRhieChowInterpolator");
    assignBlocks(params, _blocks);
    for (unsigned int d = 0; d < _dim; ++d)
      params.set<VariableName>(u_names[d]) = _velocity_name[d];

    params.set<VariableName>("pressure") = _pressure_name;
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    unsigned short smoothing_layers = isParamValid("porosity_smoothing_layers")
                                          ? getParam<unsigned short>("porosity_smoothing_layers")
                                          : 0;
    params.set<unsigned short>("smoothing_layers") = smoothing_layers;
    _problem->addUserObject("PINSFVRhieChowInterpolator", "pins_rhie_chow_interpolator", params);
  }
  else
  {
    auto params = _factory.getValidParams("INSFVRhieChowInterpolator");
    assignBlocks(params, _blocks);
    for (unsigned int d = 0; d < _dim; ++d)
      params.set<VariableName>(u_names[d]) = _velocity_name[d];

    params.set<VariableName>("pressure") = _pressure_name;
    // Set RhieChow coefficients
    if (!_has_flow_equations)
    {
      checkRhieChowFunctorsDefined();
      params.set<MooseFunctorName>("a_u") = "ax";
      params.set<MooseFunctorName>("a_v") = "ay";
      params.set<MooseFunctorName>("a_w") = "az";
    }

    _problem->addUserObject("INSFVRhieChowInterpolator", "ins_rhie_chow_interpolator", params);
  }
}

void
NSFVAction::addINSInitialConditions()
{
  // do not set initial conditions if we load from file
  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return;

  InputParameters params = _factory.getValidParams("FunctionIC");
  auto vvalue = getParam<std::vector<FunctionName>>("initial_velocity");

  if (_create_velocity)
    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<VariableName>("variable") = _velocity_name[d];
      params.set<FunctionName>("function") = vvalue[d];

      _problem->addInitialCondition("FunctionIC", _velocity_name[d] + "_ic", params);
    }

  if (_create_pressure)
  {
    params.set<VariableName>("variable") = _pressure_name;
    params.set<FunctionName>("function") = getParam<FunctionName>("initial_pressure");

    _problem->addInitialCondition("FunctionIC", _pressure_name + "_ic", params);
  }
  if (_has_energy_equation)
  {
    if (_create_fluid_temperature)
    {
      params.set<VariableName>("variable") = _fluid_temperature_name;
      params.set<FunctionName>("function") = getParam<FunctionName>("initial_temperature");

      _problem->addInitialCondition("FunctionIC", _fluid_temperature_name + "_ic", params);
    }
  }

  if (_has_scalar_equation)
  {
    unsigned int ic_counter = 0;
    for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
    {
      bool initialize_me = true;
      if (_create_scalar_variable.size())
        if (!_create_scalar_variable[name_i])
          initialize_me = false;

      if (initialize_me)
      {
        params.set<VariableName>("variable") = _passive_scalar_names[name_i];
        if (isParamValid("initial_scalar_variables"))
          params.set<FunctionName>("function") =
              getParam<std::vector<FunctionName>>("initial_scalar_variables")[ic_counter];
        else
          params.set<FunctionName>("function") = "0.0";

        _problem->addInitialCondition("FunctionIC", _passive_scalar_names[name_i] + "_ic", params);
        ic_counter += 1;
      }
    }
  }
}

void
NSFVAction::addINSMomentumTimeKernels()
{
  std::string kernel_type = "INSFVMomentumTimeDerivative";
  std::string kernel_name = "ins_momentum_time_";
  std::string rhie_chow_name = "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumTimeDerivative";
    kernel_name = "pins_momentum_time_";
    rhie_chow_name = "pins_rhie_chow_interpolator";
  }

  InputParameters params = _factory.getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.set<NonlinearVariableName>("variable") = _velocity_name[d];
    params.set<MooseEnum>("momentum_component") = NS::directions[d];

    _problem->addFVKernel(kernel_type, kernel_name + _velocity_name[d], params);
  }
}

void
NSFVAction::addINSEnergyTimeKernels()
{
  std::string kernel_type = "INSFVEnergyTimeDerivative";
  std::string kernel_name = "ins_energy_time";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVEnergyTimeDerivative";
    kernel_name = "pins_energy_time";
  }

  InputParameters params = _factory.getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;

  if (_porous_medium_treatment)
  {
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    if (_problem->hasFunctor(NS::time_deriv(_density_name), /*thread_id=*/0))
      params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
    params.set<bool>("is_solid") = false;
  }

  _problem->addFVKernel(kernel_type, kernel_name, params);
}

void
NSFVAction::addScalarTimeKernels()
{
  for (const auto & vname : _passive_scalar_names)
  {
    const std::string kernel_type = "FVFunctorTimeKernel";
    InputParameters params = _factory.getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<NonlinearVariableName>("variable") = vname;

    _problem->addFVKernel(kernel_type, "ins_" + vname + "_time", params);
  }
}

void
NSFVAction::addINSMassKernels()
{
  std::string kernel_type = "INSFVMassAdvection";
  std::string kernel_name = "ins_mass_advection";
  std::string rhie_chow_name = "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMassAdvection";
    kernel_name = "pins_mass_advection";
    rhie_chow_name = "pins_rhie_chow_interpolator";
  }

  InputParameters params = _factory.getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _pressure_name;
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.set<MooseEnum>("advected_interp_method") = _mass_advection_interpolation;

  _problem->addFVKernel(kernel_type, kernel_name, params);

  if (getParam<bool>("pin_pressure"))
  {
    MooseEnum pin_type = getParam<MooseEnum>("pinned_pressure_type");
    std::string kernel_type;
    if (pin_type == "point-value")
      kernel_type = "FVPointValueConstraint";
    else
      kernel_type = "FVIntegralValueConstraint";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<CoupledName>("lambda") = {"lambda"};
    params.set<PostprocessorName>("phi0") = getParam<PostprocessorName>("pinned_pressure_value");
    params.set<NonlinearVariableName>("variable") = _pressure_name;
    if (pin_type == "point-value")
      params.set<Point>("point") = getParam<Point>("pinned_pressure_point");

    _problem->addFVKernel(kernel_type, "ins_mass_pressure_constraint", params);
  }
}

void
NSFVAction::addINSMomentumAdvectionKernels()
{
  std::string kernel_type = "INSFVMomentumAdvection";
  std::string kernel_name = "ins_momentum_advection_";
  std::string rhie_chow_name = "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumAdvection";
    kernel_name = "pins_momentum_advection_";
    rhie_chow_name = "pins_rhie_chow_interpolator";
  }

  InputParameters params = _factory.getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.set<MooseEnum>("advected_interp_method") = _momentum_advection_interpolation;
  if (_porous_medium_treatment)
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.set<NonlinearVariableName>("variable") = _velocity_name[d];
    params.set<MooseEnum>("momentum_component") = NS::directions[d];

    _problem->addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
NSFVAction::addINSMomentumViscousDissipationKernels()
{
  std::string kernel_type = "INSFVMomentumDiffusion";
  std::string kernel_name = "ins_momentum_diffusion_";
  std::string rhie_chow_name = "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumDiffusion";
    kernel_name = "pins_momentum_diffusion_";
    rhie_chow_name = "pins_rhie_chow_interpolator";
  }

  InputParameters params = _factory.getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;

  if (_porous_medium_treatment)
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.set<NonlinearVariableName>("variable") = _velocity_name[d];
    params.set<MooseEnum>("momentum_component") = NS::directions[d];

    _problem->addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
NSFVAction::addINSMomentumMixingLengthKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  const std::string kernel_type = "INSFVMixingLengthReynoldsStress";
  InputParameters params = _factory.getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;

  std::string kernel_name = "ins_momentum_mixing_length_reynolds_stress_";
  std::string rhie_chow_name = "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_name = "pins_momentum_mixing_length_reynolds_stress_";
    rhie_chow_name = "pins_rhie_chow_interpolator";
  }

  params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  for (unsigned int dim_i = 0; dim_i < _dim; ++dim_i)
    params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_name[dim_i];

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.set<NonlinearVariableName>("variable") = _velocity_name[d];
    params.set<MooseEnum>("momentum_component") = NS::directions[d];

    _problem->addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }

  const std::string ml_kernel_type = "WallDistanceMixingLengthAux";
  InputParameters ml_params = _factory.getValidParams(ml_kernel_type);
  assignBlocks(ml_params, _blocks);
  ml_params.set<AuxVariableName>("variable") = NS::mixing_length;
  ml_params.set<std::vector<BoundaryName>>("walls") =
      getParam<std::vector<BoundaryName>>("mixing_length_walls");
  if (isParamValid("mixing_length_aux_execute_on"))
    ml_params.set<ExecFlagEnum>("execute_on") =
        getParam<ExecFlagEnum>("mixing_length_aux_execute_on");
  else
    ml_params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  ml_params.set<Real>("von_karman_const") = getParam<Real>("von_karman_const");
  ml_params.set<Real>("von_karman_const_0") = getParam<Real>("von_karman_const_0");
  ml_params.set<Real>("delta") = getParam<Real>("mixing_length_delta");

  _problem->addAuxKernel(ml_kernel_type, "mixing_length_aux ", ml_params);
}

void
NSFVAction::addINSMomentumPressureKernels()
{
  std::string kernel_type = "INSFVMomentumPressure";
  std::string kernel_name = "ins_momentum_pressure_";
  std::string rhie_chow_name = "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumPressure";
    kernel_name = "pins_momentum_pressure_";
    rhie_chow_name = "pins_rhie_chow_interpolator";
  }

  InputParameters params = _factory.getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.set<MooseFunctorName>("pressure") = _pressure_name;
  params.set<bool>("correct_skewness") =
      getParam<MooseEnum>("pressure_face_interpolation") == "skewness-corrected";
  if (_porous_medium_treatment)
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    params.set<NonlinearVariableName>("variable") = _velocity_name[d];
    _problem->addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

void
NSFVAction::addINSMomentumGravityKernels()
{
  if (isParamValid("gravity"))
  {
    std::string kernel_type = "INSFVMomentumGravity";
    std::string kernel_name = "ins_momentum_gravity_";
    std::string rhie_chow_name = "ins_rhie_chow_interpolator";

    if (_porous_medium_treatment)
    {
      kernel_type = "PINSFVMomentumGravity";
      kernel_name = "pins_momentum_gravity_";
      rhie_chow_name = "pins_rhie_chow_interpolator";
    }

    InputParameters params = _factory.getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
    if (_porous_medium_treatment)
      params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      if (getParam<RealVectorValue>("gravity")(d) != 0)
      {
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        params.set<NonlinearVariableName>("variable") = _velocity_name[d];

        _problem->addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
      }
    }
  }
}

void
NSFVAction::addINSMomentumBoussinesqKernels()
{
  if (isParamValid("gravity"))
  {
    std::string kernel_type = "INSFVMomentumBoussinesq";
    std::string kernel_name = "ins_momentum_boussinesq_";
    std::string rhie_chow_name = "ins_rhie_chow_interpolator";

    if (_porous_medium_treatment)
    {
      kernel_type = "PINSFVMomentumBoussinesq";
      kernel_name = "pins_momentum_boussinesq_";
      rhie_chow_name = "pins_rhie_chow_interpolator";
    }

    InputParameters params = _factory.getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
    params.set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
    params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
    params.set<MooseFunctorName>("alpha_name") = _thermal_expansion_name;
    if (_porous_medium_treatment)
      params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      params.set<NonlinearVariableName>("variable") = _velocity_name[d];

      _problem->addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
    }
  }
}

void
NSFVAction::addINSMomentumFrictionKernels()
{
  unsigned int num_friction_blocks = _friction_blocks.size();
  unsigned int num_used_blocks = num_friction_blocks ? num_friction_blocks : 1;

  if (_porous_medium_treatment)
  {
    const std::string kernel_type = "PINSFVMomentumFriction";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

    for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
    {
      std::string block_name = "";
      if (num_friction_blocks)
      {
        params.set<std::vector<SubdomainName>>("block") = _friction_blocks[block_i];
        block_name = Moose::stringify(_friction_blocks[block_i]);
      }
      else
      {
        assignBlocks(params, _blocks);
        block_name = std::to_string(block_i);
      }

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        for (unsigned int type_i = 0; type_i < _friction_types[block_i].size(); ++type_i)
        {
          const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
          if (upper_name == "DARCY")
            params.set<MooseFunctorName>("Darcy_name") = _friction_coeffs[block_i][type_i];
          else if (upper_name == "FORCHHEIMER")
            params.set<MooseFunctorName>("Forchheimer_name") = _friction_coeffs[block_i][type_i];
        }

        _problem->addFVKernel(
            kernel_type, "momentum_friction_" + block_name + "_" + NS::directions[d], params);
      }

      if (_use_friction_correction)
      {
        const std::string correction_kernel_type = "PINSFVMomentumFrictionCorrection";
        InputParameters corr_params = _factory.getValidParams(correction_kernel_type);
        if (num_friction_blocks)
          corr_params.set<std::vector<SubdomainName>>("block") = _friction_blocks[block_i];
        else
          assignBlocks(corr_params, _blocks);
        corr_params.set<MooseFunctorName>(NS::density) = _density_name;
        corr_params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
        corr_params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
        corr_params.set<Real>("consistent_scaling") = getParam<Real>("consistent_scaling");
        for (unsigned int d = 0; d < _dim; ++d)
        {
          corr_params.set<NonlinearVariableName>("variable") = _velocity_name[d];
          corr_params.set<MooseEnum>("momentum_component") = NS::directions[d];
          for (unsigned int type_i = 0; type_i < _friction_types[block_i].size(); ++type_i)
          {
            const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
            if (upper_name == "DARCY")
              corr_params.set<MooseFunctorName>("Darcy_name") = _friction_coeffs[block_i][type_i];
            else if (upper_name == "FORCHHEIMER")
              corr_params.set<MooseFunctorName>("Forchheimer_name") =
                  _friction_coeffs[block_i][type_i];
          }

          _problem->addFVKernel(correction_kernel_type,
                                "pins_momentum_friction_correction_" + block_name + "_" +
                                    NS::directions[d],
                                corr_params);
        }
      }
    }
  }
  else
  {
    const std::string kernel_type = "INSFVMomentumFriction";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";

    for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
    {
      std::string block_name = "";
      if (num_friction_blocks)
      {
        params.set<std::vector<SubdomainName>>("block") = _friction_blocks[block_i];
        block_name = Moose::stringify(_friction_blocks[block_i]);
      }
      else
      {
        assignBlocks(params, _blocks);
        block_name = std::to_string(block_i);
      }

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        for (unsigned int type_i = 0; type_i < _friction_types[block_i].size(); ++type_i)
        {
          const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
          if (upper_name == "DARCY")
            params.set<MooseFunctorName>("linear_coef_name") = _friction_coeffs[block_i][type_i];
          else if (upper_name == "FORCHHEIMER")
            params.set<MooseFunctorName>("quadratic_coef_name") = _friction_coeffs[block_i][type_i];
        }

        _problem->addFVKernel(
            kernel_type, "ins_momentum_friction_" + block_name + "_" + NS::directions[d], params);
      }
    }
  }
}

void
NSFVAction::addINSEnergyAdvectionKernels()
{
  std::string kernel_type = "INSFVEnergyAdvection";
  std::string kernel_name = "ins_energy_advection";
  std::string rhie_chow_name = "ins_rhie_chow_interpolator";
  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVEnergyAdvection";
    kernel_name = "pins_energy_advection";
    rhie_chow_name = "pins_rhie_chow_interpolator";
  }

  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  assignBlocks(params, _blocks);
  params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.set<MooseEnum>("advected_interp_method") = _energy_advection_interpolation;

  _problem->addFVKernel(kernel_type, kernel_name, params);
}

void
NSFVAction::addINSEnergyHeatConductionKernels()
{
  bool vector_conductivity = processThermalConductivity();
  unsigned int num_blocks = _thermal_conductivity_blocks.size();
  unsigned int num_used_blocks = num_blocks ? num_blocks : 1;

  for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
  {
    std::string block_name = "";
    if (num_blocks)
      block_name = Moose::stringify(_thermal_conductivity_blocks[block_i]);
    else
      block_name = std::to_string(block_i);

    if (_porous_medium_treatment)
    {
      const std::string kernel_type =
          vector_conductivity ? "PINSFVEnergyAnisotropicDiffusion" : "PINSFVEnergyDiffusion";

      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      std::vector<SubdomainName> block_names =
          num_blocks ? _thermal_conductivity_blocks[block_i] : _blocks;
      assignBlocks(params, block_names);
      std::string conductivity_name = vector_conductivity ? NS::kappa : NS::k;
      params.set<MooseFunctorName>(conductivity_name) = _thermal_conductivity_name[block_i];
      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;

      _problem->addFVKernel(kernel_type, "pins_energy_diffusion_" + block_name, params);
    }
    else
    {
      const std::string kernel_type = "FVDiffusion";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      std::vector<SubdomainName> block_names =
          num_blocks ? _thermal_conductivity_blocks[block_i] : _blocks;
      assignBlocks(params, block_names);
      params.set<MooseFunctorName>("coeff") = _thermal_conductivity_name[block_i];

      _problem->addFVKernel(kernel_type, "ins_energy_diffusion_" + block_name, params);
    }
  }
}

void
NSFVAction::addINSEnergyAmbientConvection()
{
  unsigned int num_convection_blocks = _ambient_convection_blocks.size();
  unsigned int num_used_blocks = num_convection_blocks ? num_convection_blocks : 1;

  const std::string kernel_type = "PINSFVEnergyAmbientConvection";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;
  params.set<bool>("is_solid") = false;

  for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
  {
    std::string block_name = "";
    if (num_convection_blocks)
    {
      params.set<std::vector<SubdomainName>>("block") = _ambient_convection_blocks[block_i];
      block_name = Moose::stringify(_ambient_convection_blocks[block_i]);
    }
    else
    {
      assignBlocks(params, _blocks);
      block_name = std::to_string(block_i);
    }

    params.set<MooseFunctorName>("h_solid_fluid") = _ambient_convection_alpha[block_i];
    params.set<MooseFunctorName>(NS::T_solid) = _ambient_temperature[block_i];

    _problem->addFVKernel(kernel_type, "ambient_convection_" + block_name, params);
  }
}

void
NSFVAction::addINSEnergyExternalHeatSource()
{
  const std::string kernel_type = "FVCoupledForce";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>("v") = getParam<MooseFunctorName>("external_heat_source");
  params.set<Real>("coef") = getParam<Real>("external_heat_source_coeff");

  _problem->addFVKernel(kernel_type, "external_heat_source", params);
}

void
NSFVAction::addScalarAdvectionKernels()
{
  for (const auto & vname : _passive_scalar_names)
  {
    const std::string kernel_type = "INSFVScalarFieldAdvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = vname;
    params.set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
    params.set<MooseEnum>("advected_interp_method") = _passive_scalar_advection_interpolation;

    if (_porous_medium_treatment)
      params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
    else
      params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";

    _problem->addFVKernel(kernel_type, "ins_" + vname + "_advection", params);
  }
}

void
NSFVAction::addScalarMixingLengthKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  const std::string kernel_type = "INSFVMixingLengthScalarDiffusion";
  InputParameters params = _factory.getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;

  for (unsigned int dim_i = 0; dim_i < _dim; ++dim_i)
    params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_name[dim_i];

  for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
  {
    params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
    if (_passive_scalar_schmidt_number.size())
      params.set<Real>("schmidt_number") = _passive_scalar_schmidt_number[name_i];
    else
      params.set<Real>("schmidt_number") = 1.0;

    _problem->addFVKernel(kernel_type, _passive_scalar_names[name_i] + "_mixing_length", params);
  }
}

void
NSFVAction::addScalarDiffusionKernels()
{
  for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
  {
    const std::string kernel_type = "FVDiffusion";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
    assignBlocks(params, _blocks);
    params.set<MooseFunctorName>("coeff") = _passive_scalar_diffusivity[name_i];

    _problem->addFVKernel(
        kernel_type, "ins_" + _passive_scalar_names[name_i] + "_diffusion", params);
  }
}

void
NSFVAction::addScalarSourceKernels()
{
  for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
  {
    const std::string kernel_type = "FVBodyForce";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
    assignBlocks(params, _blocks);
    params.set<FunctionName>("function") = _passive_scalar_source[name_i];

    _problem->addFVKernel(kernel_type, "ins_" + _passive_scalar_names[name_i] + "_source", params);
  }
}

void
NSFVAction::addScalarCoupledSourceKernels()
{
  for (unsigned int name_eq = 0; name_eq < _passive_scalar_names.size(); name_eq++)
  {
    for (unsigned int i = 0; i < _passive_scalar_coupled_source[name_eq].size(); ++i)
    {
      const std::string kernel_type = "FVCoupledForce";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_eq];
      assignBlocks(params, _blocks);
      params.set<MooseFunctorName>("v") = _passive_scalar_coupled_source[name_eq][i];
      params.set<Real>("coef") = _passive_scalar_coupled_source_coeff[name_eq][i];

      _problem->addFVKernel(kernel_type,
                            "ins_" + _passive_scalar_names[name_eq] + "_coupled_source_" +
                                std::to_string(i),
                            params);
    }
  }
}

void
NSFVAction::addINSInletBC()
{
  unsigned int flux_bc_counter = 0;
  unsigned int velocity_pressure_counter = 0;
  for (unsigned int bc_ind = 0; bc_ind < _inlet_boundaries.size(); ++bc_ind)
  {
    if (_momentum_inlet_types[bc_ind] == "fixed-velocity")
    {
      const std::string bc_type = "INSFVInletVelocityBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.set<FunctionName>("function") =
            _momentum_inlet_function[velocity_pressure_counter][d];

        _problem->addFVBC(bc_type, _velocity_name[d] + "_" + _inlet_boundaries[bc_ind], params);
      }
      ++velocity_pressure_counter;
    }
    else if (_momentum_inlet_types[bc_ind] == "fixed-pressure")
    {
      const std::string bc_type = "INSFVOutletPressureBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _pressure_name;
      params.set<FunctionName>("function") = _momentum_inlet_function[velocity_pressure_counter][0];
      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      _problem->addFVBC(bc_type, _pressure_name + "_" + _inlet_boundaries[bc_ind], params);
      ++velocity_pressure_counter;
    }
    else if (_momentum_inlet_types[bc_ind] == "flux-mass" ||
             _momentum_inlet_types[bc_ind] == "flux-velocity")
    {
      {
        const std::string bc_type =
            _porous_medium_treatment ? "PWCNSFVMomentumFluxBC" : "WCNSFVMomentumFluxBC";
        InputParameters params = _factory.getValidParams(bc_type);

        if (_flux_inlet_directions.size())
          params.set<Point>("direction") = _flux_inlet_directions[flux_bc_counter];

        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};
        if (_porous_medium_treatment)
        {
          params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
          params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
        }
        else
          params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";

        if (_momentum_inlet_types[bc_ind] == "flux-mass")
        {
          params.set<PostprocessorName>("mdot_pp") = _flux_inlet_pps[flux_bc_counter];
          params.set<PostprocessorName>("area_pp") = "area_pp_" + _inlet_boundaries[bc_ind];
        }
        else
          params.set<PostprocessorName>("velocity_pp") = _flux_inlet_pps[flux_bc_counter];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          params.set<NonlinearVariableName>("variable") = _velocity_name[d];

          _problem->addFVBC(bc_type, _velocity_name[d] + "_" + _inlet_boundaries[bc_ind], params);
        }
      }
      {
        const std::string bc_type = "WCNSFVMassFluxBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<NonlinearVariableName>("variable") = _pressure_name;
        params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

        if (_flux_inlet_directions.size())
          params.set<Point>("direction") = _flux_inlet_directions[flux_bc_counter];

        if (_momentum_inlet_types[bc_ind] == "flux-mass")
        {
          params.set<PostprocessorName>("mdot_pp") = _flux_inlet_pps[flux_bc_counter];
          params.set<PostprocessorName>("area_pp") = "area_pp_" + _inlet_boundaries[bc_ind];
        }
        else
          params.set<PostprocessorName>("velocity_pp") = _flux_inlet_pps[flux_bc_counter];

        _problem->addFVBC(bc_type, _pressure_name + "_" + _inlet_boundaries[bc_ind], params);
      }

      // need to increment flux_bc_counter
      ++flux_bc_counter;
    }
  }
}

void
NSFVAction::addINSEnergyInletBC()
{
  unsigned int flux_bc_counter = 0;
  for (unsigned int bc_ind = 0; bc_ind < _inlet_boundaries.size(); ++bc_ind)
  {
    if (_energy_inlet_types[bc_ind] == "fixed-temperature")
    {
      const std::string bc_type = "FVFunctionDirichletBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<FunctionName>("function") = _energy_inlet_function[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      _problem->addFVBC(bc_type, _fluid_temperature_name + "_" + _inlet_boundaries[bc_ind], params);
    }
    else if (_energy_inlet_types[bc_ind] == "heatflux")
    {
      const std::string bc_type = "FVFunctionNeumannBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<FunctionName>("function") = _energy_inlet_function[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      _problem->addFVBC(bc_type, _fluid_temperature_name + "_" + _inlet_boundaries[bc_ind], params);
    }
    else if (_energy_inlet_types[bc_ind] == "flux-mass" ||
             _energy_inlet_types[bc_ind] == "flux-velocity")
    {
      const std::string bc_type = "WCNSFVEnergyFluxBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      if (_flux_inlet_directions.size())
        params.set<Point>("direction") = _flux_inlet_directions[flux_bc_counter];
      if (_energy_inlet_types[bc_ind] == "flux-mass")
      {
        params.set<PostprocessorName>("mdot_pp") = _flux_inlet_pps[flux_bc_counter];
        params.set<PostprocessorName>("area_pp") = "area_pp_" + _inlet_boundaries[bc_ind];
      }
      else
        params.set<PostprocessorName>("velocity_pp") = _flux_inlet_pps[flux_bc_counter];

      params.set<PostprocessorName>("temperature_pp") = _energy_inlet_function[bc_ind];
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;

      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      _problem->addFVBC(bc_type, _fluid_temperature_name + "_" + _inlet_boundaries[bc_ind], params);
      flux_bc_counter += 1;
    }
  }
}

void
NSFVAction::addScalarInletBC()
{
  for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
  {
    unsigned int flux_bc_counter = 0;
    unsigned int num_inlets = _inlet_boundaries.size();
    for (unsigned int bc_ind = 0; bc_ind < num_inlets; ++bc_ind)
    {
      if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "fixed-value")
      {
        const std::string bc_type = "FVFunctionDirichletBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
        params.set<FunctionName>("function") = _passive_scalar_inlet_function[name_i][bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

        _problem->addFVBC(
            bc_type, _passive_scalar_names[name_i] + "_" + _inlet_boundaries[bc_ind], params);
      }
      else if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-mass" ||
               _passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-velocity")
      {
        const std::string bc_type = "WCNSFVScalarFluxBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
        if (_flux_inlet_directions.size())
          params.set<Point>("direction") = _flux_inlet_directions[flux_bc_counter];
        if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-mass")
        {
          params.set<PostprocessorName>("mdot_pp") = _flux_inlet_pps[flux_bc_counter];
          params.set<PostprocessorName>("area_pp") = "area_pp_" + _inlet_boundaries[bc_ind];
          params.set<MooseFunctorName>(NS::density) = _density_name;
        }
        else
          params.set<PostprocessorName>("velocity_pp") = _flux_inlet_pps[flux_bc_counter];

        params.set<PostprocessorName>("scalar_value_pp") =
            _passive_scalar_inlet_function[name_i][bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

        _problem->addFVBC(
            bc_type, _passive_scalar_names[name_i] + "_" + _inlet_boundaries[bc_ind], params);
        flux_bc_counter += 1;
      }
    }
  }
}

void
NSFVAction::addINSOutletBC()
{
  const std::string u_names[3] = {"u", "v", "w"};
  for (unsigned int bc_ind = 0; bc_ind < _outlet_boundaries.size(); ++bc_ind)
  {

    if (_momentum_outlet_types[bc_ind] == "zero-gradient" ||
        _momentum_outlet_types[bc_ind] == "fixed-pressure-zero-gradient")
    {
      if (_porous_medium_treatment)
      {
        const std::string bc_type = "PINSFVMomentumAdvectionOutflowBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};
        params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
        params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
        params.set<MooseFunctorName>(NS::density) = _density_name;

        for (unsigned int i = 0; i < _dim; ++i)
          params.set<MooseFunctorName>(u_names[i]) = _velocity_name[i];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          params.set<NonlinearVariableName>("variable") = _velocity_name[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];

          _problem->addFVBC(bc_type, _velocity_name[d] + "_" + _outlet_boundaries[bc_ind], params);
        }
      }
      else
      {
        const std::string bc_type = "INSFVMomentumAdvectionOutflowBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};
        params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
        params.set<MooseFunctorName>(NS::density) = _density_name;

        for (unsigned int i = 0; i < _dim; ++i)
          params.set<MooseFunctorName>(u_names[i]) = _velocity_name[i];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          params.set<NonlinearVariableName>("variable") = _velocity_name[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];

          _problem->addFVBC(bc_type, _velocity_name[d] + "_" + _outlet_boundaries[bc_ind], params);
        }
      }
    }

    if (_momentum_outlet_types[bc_ind] == "fixed-pressure" ||
        _momentum_outlet_types[bc_ind] == "fixed-pressure-zero-gradient")
    {
      const std::string bc_type = "INSFVOutletPressureBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _pressure_name;
      params.set<FunctionName>("function") = _pressure_function[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

      _problem->addFVBC(bc_type, _pressure_name + "_" + _outlet_boundaries[bc_ind], params);
    }
    else if (_momentum_outlet_types[bc_ind] == "zero-gradient")
    {
      const std::string bc_type = "INSFVMassAdvectionOutflowBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _pressure_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
        params.set<MooseFunctorName>(u_names[d]) = _velocity_name[d];

      _problem->addFVBC(bc_type, _pressure_name + "_" + _outlet_boundaries[bc_ind], params);
    }
  }
}

void
NSFVAction::addINSWallBC()
{
  const std::string u_names[3] = {"u", "v", "w"};

  for (unsigned int bc_ind = 0; bc_ind < _wall_boundaries.size(); ++bc_ind)
  {
    if (_momentum_wall_types[bc_ind] == "noslip")
    {
      const std::string bc_type = "INSFVNoSlipWallBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.set<FunctionName>("function") = "0";

        _problem->addFVBC(bc_type, _velocity_name[d] + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (_momentum_wall_types[bc_ind] == "wallfunction")
    {
      const std::string bc_type = "INSFVWallFunctionBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      if (_porous_medium_treatment)
        params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
      else
        params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";

      for (unsigned int d = 0; d < _dim; ++d)
        params.set<MooseFunctorName>(u_names[d]) = _velocity_name[d];

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];

        _problem->addFVBC(bc_type, _velocity_name[d] + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (_momentum_wall_types[bc_ind] == "slip")
    {
      const std::string bc_type = "INSFVNaturalFreeSlipBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        if (_porous_medium_treatment)
          params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
        else
          params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";

        params.set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];

        _problem->addFVBC(bc_type, _velocity_name[d] + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (_momentum_wall_types[bc_ind] == "symmetry")
    {
      {
        std::string bc_type;
        if (_porous_medium_treatment)
          bc_type = "PINSFVSymmetryVelocityBC";
        else
          bc_type = "INSFVSymmetryVelocityBC";

        InputParameters params = _factory.getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

        MooseFunctorName viscosity_name = _dynamic_viscosity_name;
        if (_turbulence_handling != "none")
          viscosity_name = NS::total_viscosity;
        params.set<MooseFunctorName>(NS::mu) = viscosity_name;

        for (unsigned int d = 0; d < _dim; ++d)
          params.set<MooseFunctorName>(u_names[d]) = _velocity_name[d];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          if (_porous_medium_treatment)
            params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
          else
            params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";

          params.set<NonlinearVariableName>("variable") = _velocity_name[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];

          _problem->addFVBC(bc_type, _velocity_name[d] + "_" + _wall_boundaries[bc_ind], params);
        }
      }
      {
        const std::string bc_type = "INSFVSymmetryPressureBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = _pressure_name;
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

        _problem->addFVBC(bc_type, _pressure_name + "_" + _wall_boundaries[bc_ind], params);
      }
    }
  }
}

void
NSFVAction::addINSEnergyWallBC()
{
  for (unsigned int bc_ind = 0; bc_ind < _wall_boundaries.size(); ++bc_ind)
  {
    if (_energy_wall_types[bc_ind] == "fixed-temperature")
    {
      const std::string bc_type = "FVFunctionDirichletBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<FunctionName>("function") = _energy_wall_function[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      _problem->addFVBC(bc_type, _fluid_temperature_name + "_" + _wall_boundaries[bc_ind], params);
    }
    else if (_energy_wall_types[bc_ind] == "heatflux")
    {
      const std::string bc_type = "FVFunctionNeumannBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.set<FunctionName>("function") = _energy_wall_function[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      _problem->addFVBC(bc_type, _fluid_temperature_name + "_" + _wall_boundaries[bc_ind], params);
    }
  }
}

void
NSFVAction::addWCNSMassTimeKernels()
{
  std::string mass_kernel_type = "WCNSFVMassTimeDerivative";
  std::string kernel_name = "wcns_mass_time";

  if (_porous_medium_treatment)
  {
    mass_kernel_type = "PWCNSFVMassTimeDerivative";
    kernel_name = "pwcns_mass_time";
  }

  InputParameters params = _factory.getValidParams(mass_kernel_type);
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _pressure_name;
  params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
  if (_porous_medium_treatment)
    params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  _problem->addFVKernel(mass_kernel_type, kernel_name, params);
}

void
NSFVAction::addWCNSMomentumTimeKernels()
{
  const std::string mom_kernel_type = "WCNSFVMomentumTimeDerivative";
  InputParameters params = _factory.getValidParams(mom_kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.set<MooseEnum>("momentum_component") = NS::directions[d];
    params.set<NonlinearVariableName>("variable") = _velocity_name[d];

    if (_porous_medium_treatment)
    {
      params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
      _problem->addFVKernel(
          mom_kernel_type, "pwcns_momentum_" + NS::directions[d] + "_time", params);
    }
    else
    {
      params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
      _problem->addFVKernel(
          mom_kernel_type, "wcns_momentum_" + NS::directions[d] + "_time", params);
    }
  }
}

void
NSFVAction::addWCNSEnergyTimeKernels()
{
  std::string en_kernel_type = "WCNSFVEnergyTimeDerivative";
  std::string kernel_name = "wcns_energy_time";

  if (_porous_medium_treatment)
  {
    en_kernel_type = "PINSFVEnergyTimeDerivative";
    kernel_name = "pwcns_energy_time";
  }

  InputParameters params = _factory.getValidParams(en_kernel_type);
  assignBlocks(params, _blocks);
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
  params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;

  if (_porous_medium_treatment)
  {
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    params.set<bool>("is_solid") = false;
  }

  _problem->addFVKernel(en_kernel_type, kernel_name, params);
}

void
NSFVAction::addWCNSEnergyMixingLengthKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  const std::string kernel_type = "WCNSFVMixingLengthEnergyDiffusion";
  InputParameters params = _factory.getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
  params.set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;
  params.set<Real>("schmidt_number") = getParam<Real>("turbulent_prandtl");
  params.set<NonlinearVariableName>("variable") = _fluid_temperature_name;

  for (unsigned int dim_i = 0; dim_i < _dim; ++dim_i)
    params.set<MooseFunctorName>(u_names[dim_i]) = _velocity_name[dim_i];

  if (_porous_medium_treatment)
    _problem->addFVKernel(kernel_type, "pins_energy_mixing_length_diffusion", params);
  else
    _problem->addFVKernel(kernel_type, "ins_energy_mixing_length_diffusion", params);
}

void
NSFVAction::addEnthalpyMaterial()
{
  InputParameters params = _factory.getValidParams("INSFVEnthalpyMaterial");
  assignBlocks(params, _blocks);

  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
  params.set<MooseFunctorName>("temperature") = _fluid_temperature_name;

  _problem->addMaterial("INSFVEnthalpyMaterial", "ins_enthalpy_material", params);
}

void
NSFVAction::addPorousMediumSpeedMaterial()
{
  InputParameters params = _factory.getValidParams("PINSFVSpeedFunctorMaterial");
  assignBlocks(params, _blocks);

  for (unsigned int dim_i = 0; dim_i < _dim; ++dim_i)
    params.set<MooseFunctorName>(NS::superficial_velocity_vector[dim_i]) = _velocity_name[dim_i];
  params.set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  _problem->addMaterial("PINSFVSpeedFunctorMaterial", "pins_speed_material", params);
}

void
NSFVAction::addMixingLengthMaterial()
{
  const std::string u_names[3] = {"u", "v", "w"};
  InputParameters params = _factory.getValidParams("MixingLengthTurbulentViscosityMaterial");
  assignBlocks(params, _blocks);

  for (unsigned int d = 0; d < _dim; ++d)
    params.set<MooseFunctorName>(u_names[d]) = _velocity_name[d];

  params.set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;

  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;

  _problem->addMaterial("MixingLengthTurbulentViscosityMaterial", "mixing_length_material", params);
}

void
NSFVAction::addBoundaryPostprocessors()
{
  for (unsigned int bc_ind = 0; bc_ind < _inlet_boundaries.size(); ++bc_ind)
    if (_momentum_inlet_types[bc_ind] == "flux-mass" ||
        (_has_energy_equation && _momentum_inlet_types[bc_ind] == "flux-velocity"))
    {
      const std::string pp_type = "AreaPostprocessor";
      InputParameters params = _factory.getValidParams(pp_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};
      params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

      _problem->addPostprocessor(pp_type, "area_pp_" + _inlet_boundaries[bc_ind], params);
    }
}

void
NSFVAction::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  unsigned short necessary_layers = 2;
  if (_momentum_face_interpolation == "skewness-corrected" ||
      _energy_face_interpolation == "skewness-corrected" ||
      _pressure_face_interpolation == "skewness-corrected" ||
      _turbulence_handling == "mixing-length" ||
      (_porous_medium_treatment &&
       getParam<MooseEnum>("porosity_interface_pressure_treatment") != "automatic"))
    necessary_layers = 3;

  if (_porous_medium_treatment && isParamValid("porosity_smoothing_layers"))
    necessary_layers =
        std::max(getParam<unsigned short>("porosity_smoothing_layers"), necessary_layers);

  const std::string kernel_type = "INSFVMixingLengthReynoldsStress";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<unsigned short>("ghost_layers") = necessary_layers;
  addRelationshipManagers(input_rm_type, params);
}

void
NSFVAction::processMesh()
{
  _dim = _mesh->dimension();
  _problem->needFV();

  // If the user doesn't define a block name we go with the default
  if (!_blocks.size())
    _blocks.push_back("ANY_BLOCK_ID");
}

void
NSFVAction::assignBlocks(InputParameters & params, const std::vector<SubdomainName> & blocks)
{
  // We only set the blocks if we don't have `ANY_BLOCK_ID` defined because the subproblem
  // (throug the mesh) errors out if we use this keyword during the addVariable/Kernel
  // functions
  if (std::find(blocks.begin(), blocks.end(), "ANY_BLOCK_ID") == blocks.end())
    params.set<std::vector<SubdomainName>>("block") = blocks;
}

void
NSFVAction::processVariables()
{
  _create_scalar_variable.clear();
  for (const auto & it : _passive_scalar_names)
    if (_problem->hasVariable(it))
      _create_scalar_variable.push_back(false);
    else
      _create_scalar_variable.push_back(true);

  if ((_velocity_name.size() != _dim) && (_velocity_name.size() != 3))
    mooseError("The number of velocity variable names supplied to the NSFVAction is not " +
               std::to_string(_dim) + " (mesh dimension) or 3!");

  if (!_create_velocity)
    for (const auto & vname : _velocity_name)
    {
      if (!(_problem->hasVariable(vname)))
        paramError("velocity_variable",
                   "Variable (" + vname +
                       ") supplied to the NavierStokesFV action does not exist!");
      else
        checkVariableBlockConsistency(vname);
    }

  if (!_create_pressure)
  {
    if (!(_problem->hasVariable(_pressure_name)))
      paramError("pressure_variable",
                 "Variable (" + _pressure_name +
                     ") supplied to the NavierStokesFV action does not exist!");
    else
      checkVariableBlockConsistency(_pressure_name);
  }

  if (!_create_fluid_temperature)
    if (!(_problem->hasVariable(_fluid_temperature_name)))
      paramError("pressure_variable",
                 "Variable (" + _fluid_temperature_name +
                     ") supplied to the NavierStokesFV action does not exist!");
}

void
NSFVAction::checkVariableBlockConsistency(const std::string & var_name)
{
  const auto & fv_variable =
      dynamic_cast<const MooseVariableFVReal &>(_problem->getVariable(0, var_name));
  const auto & variable_blocks = fv_variable.blocks();

  std::vector<SubdomainName> real_action_block_names =
      _blocks.size() ? _blocks : std::vector<SubdomainName>({"ANY_BLOCK_ID"});

  for (const auto & action_block : real_action_block_names)
  {
    if (std::find(variable_blocks.begin(), variable_blocks.end(), action_block) ==
        variable_blocks.end())
      paramError("block",
                 "The suppled variable (",
                 var_name,
                 ") does not have the same block-restriction as the NSFVAction. The restriction of "
                 "the variable is: (",
                 Moose::stringify(variable_blocks),
                 ") while the restriction for the action is: (",
                 Moose::stringify(_blocks),
                 ")");
  }
}

bool
NSFVAction::processThermalConductivity()
{
  checkBlockwiseConsistency<MooseFunctorName>("thermal_conductivity_blocks",
                                              {"thermal_conductivity"});
  bool have_scalar = false;
  bool have_vector = false;

  for (unsigned int i = 0; i < _thermal_conductivity_name.size(); ++i)
  {
    // First, check if the name is just a number (only in case of isotropic conduction)
    if (MooseUtils::parsesToReal(_thermal_conductivity_name[i]))
      have_scalar = true;
    // Now we determine what kind of functor we are dealing with
    else
    {
      if (_problem->hasFunctorWithType<ADReal>(_thermal_conductivity_name[i], /*thread_id=*/0))
        have_scalar = true;
      else
      {
        if (_problem->hasFunctorWithType<ADRealVectorValue>(_thermal_conductivity_name[i],
                                                            /*thread_id=*/0))
          have_vector = true;
        else
        {
          paramError("thermal_conductivity",
                     "We only allow functor of type ADReal or ADRealVectorValue for thermal "
                     "conductivity!");
        }
      }
    }
  }

  if (have_vector && !_porous_medium_treatment)
    paramError("thermal_conductivity", "Cannot use anistropic diffusion with non-porous flows!");

  if (have_vector == have_scalar)
    paramError("thermal_conductivity",
               "The entries on thermal conductivity shall either be scalars of vectors, mixing "
               "them is not supported!");
  return have_vector;
}

void
NSFVAction::checkGeneralControlErrors()
{
  if (_compressibility == "weakly-compressible" && _boussinesq_approximation == true)
    paramError("boussinesq_approximation",
               "We cannot use boussinesq approximation while running in weakly-compressible mode!");

  if (isParamValid("porosity_smoothing_layers"))
  {
    if (!_porous_medium_treatment)
      paramError(
          "porosity_smoothing_layers",
          "This parameter should not be defined if the porous medium treatment is disabled!");

    if (getParam<unsigned short>("porosity_smoothing_layers") != 0 &&
        getParam<MooseEnum>("porosity_interface_pressure_treatment") != "automatic")
      paramError("porosity_interface_pressure_treatment",
                 "If 'porosity_smoothing_layers' is non-zero, e.g. if the porosity is smooth(ed), "
                 "then automatic pressure calculation should be used.");
  }

  if (!_porous_medium_treatment && _use_friction_correction)
    paramError("use_friction_correction",
               "This parameter should not be defined if the porous medium treatment is disabled!");

  if (_porous_medium_treatment && _has_scalar_equation)
    paramError("porous_medium_treatment",
               "Porous media scalar advection is currently unimplemented!");

  if (isParamValid("consistent_scaling") && !_use_friction_correction)
    paramError("consistent_scaling",
               "Consistent scaling should not be defined if friction correction is disabled!");

  if (getParam<bool>("pin_pressure"))
  {
    checkDependentParameterError("pin_pressure", {"pinned_pressure_type"}, true);

    MooseEnum pin_type = getParam<MooseEnum>("pinned_pressure_type");
    checkDependentParameterError(
        "pinned_pressure_type", {"pinned_pressure_point"}, pin_type == "point-value");
  }

  if (!_has_energy_equation)
    checkDependentParameterError("add_energy_equation",
                                 {"energy_inlet_types",
                                  "energy_scaling",
                                  "energy_inlet_function",
                                  "energy_face_interpolation",
                                  "energy_two_term_bc_expansion",
                                  "energy_wall_function",
                                  "energy_wall_types",
                                  "energy_advection_interpolation",
                                  "specific_heat",
                                  "thermal_conductivity_blocks",
                                  "thermal_conductivity",
                                  "use_external_enthalpy_material"});
  if (!_porous_medium_treatment)
    checkDependentParameterError(
        "porous_medium_treatment",
        {"porosity_smoothing_layers", "use_friction_correction", "consistent_scaling"});

  if (_turbulence_handling != "mixing-length")
    checkDependentParameterError("turbulence_handling",
                                 {"mixing_length_delta",
                                  "mixing_length_aux_execute_on",
                                  "mixing_length_walls",
                                  "von_karman_const",
                                  "von_karman_const_0"});

  if (!_has_scalar_equation)
    checkDependentParameterError("add_scalar_equation",
                                 {"passive_scalar_names",
                                  "passive_scalar_source",
                                  "passive_scalar_scaling",
                                  "passive_scalar_diffusivity",
                                  "passive_scalar_inlet_types",
                                  "passive_scalar_coupled_source",
                                  "passive_scalar_inlet_function",
                                  "passive_scalar_schmidt_number",
                                  "passive_scalar_face_interpolation",
                                  "passive_scalar_coupled_source_coeff",
                                  "passive_scalar_two_term_bc_expansion",
                                  "passive_scalar_advection_interpolation"});
}

void
NSFVAction::checkICParameterErrors()
{
  if (isParamValid("initial_scalar_variables"))
  {
    unsigned int num_created_variables = 0;
    for (const auto & it : _create_scalar_variable)
      if (it == true)
        num_created_variables += 1;
    auto num_init_conditions =
        getParam<std::vector<FunctionName>>("initial_scalar_variables").size();
    if (num_created_variables != num_init_conditions)
      paramError("initial_scalar_variables",
                 "The number of initial conditions (" + std::to_string(num_init_conditions) +
                     ") is not equal to the number of self-generated variables (" +
                     std::to_string(num_created_variables) + ") !");
  }

  auto vvalue = getParam<std::vector<FunctionName>>("initial_velocity");
  if (vvalue.size() != 3)
    mooseError("The number of velocity components in the NSFVAction initial condition is not 3!");

  // Dont define initial conditions if using external variables
  if (parameters().isParamSetByUser("initial_velocity") && !_create_velocity)
    paramError("initial_velocity",
               "Velocity is defined externally of NavierStokesFV, so should the inital conditions");

  if (parameters().isParamSetByUser("initial_pressure") && !_create_pressure)
    paramError("initial_pressure",
               "Pressure is defined externally of NavierStokesFV, so should the inital condition");

  if (parameters().isParamSetByUser("initial_temperature") && !_create_fluid_temperature)
    paramError("initial_temperature",
               "T_fluid is defined externally of NavierStokesFV, so should the inital condition");

  if (getParam<bool>("initialize_variables_from_mesh_file"))
    checkDependentParameterError("initialize_variables_from_mesh_file",
                                 {"initial_velocity",
                                  "initial_pressure",
                                  "initial_temperature",
                                  "initial_scalar_variables"});
}

void
NSFVAction::checkBoundaryParameterErrors()
{
  checkSizeFriendParams(_outlet_boundaries.size(),
                        _momentum_outlet_types.size(),
                        "outlet_boundaries",
                        "momentum_outlet_types",
                        "outlet boundaries");

  if (_wall_boundaries.size() > 0)
    checkSizeFriendParams(_wall_boundaries.size(),
                          _momentum_wall_types.size(),
                          "wall_boundaries",
                          "momentum_wall_types",
                          "wall boundaries");

  checkSizeFriendParams(_inlet_boundaries.size(),
                        _momentum_inlet_types.size(),
                        "inlet_boundaries",
                        "momentum_inlet_types",
                        "inlet boundaries");

  unsigned int num_dir_dependent_bcs = 0;
  unsigned int num_pressure_inlet_bcs = 0;
  unsigned int num_flux_bc_postprocessors = 0;
  for (const auto & type : _momentum_inlet_types)
  {
    if (type == "fixed-velocity")
      num_dir_dependent_bcs += 1;
    else if (type == "flux-velocity" || type == "flux-mass")
      num_flux_bc_postprocessors += 1;
    else if (type == "fixed-pressure")
      num_pressure_inlet_bcs += 1;
  }

  checkSizeParam(_momentum_inlet_function.size(),
                 "momentum_inlet_function",
                 num_dir_dependent_bcs + num_pressure_inlet_bcs,
                 "fixed-velocity and fixed-pressure entries",
                 "momentum_inlet_types");

  // index into _momentum_inlet_function
  unsigned int k = 0;
  for (const auto & type : _momentum_inlet_types)
  {
    if (type != "fixed-velocity" && type != "fixed-pressure")
      continue;

    if (type == "fixed-velocity")
      checkSizeParam(_momentum_inlet_function[k].size(),
                     "momentum_inlet_function",
                     _dim,
                     "entries ",
                     " the momentum_inlet_types subvector for fixed-velocity inlet: " +
                         _inlet_boundaries[k]);
    else if (type == "fixed-pressure")
      checkSizeParam(_momentum_inlet_function[k].size(),
                     "momentum_inlet_function",
                     1,
                     "entries ",
                     " the momentum_inlet_types subvector for fixed-pressure inlet: " +
                         _inlet_boundaries[k]);
    ++k;
  }

  checkSizeParam(_flux_inlet_pps.size(),
                 "flux_inlet_pps",
                 num_flux_bc_postprocessors,
                 "flux types",
                 "'inlet_boundaries'");

  if (_flux_inlet_directions.size())
    checkSizeParam(_flux_inlet_directions.size(),
                   "flux_inlet_directions",
                   num_flux_bc_postprocessors,
                   "flux types",
                   "'inlet_boundaries'");

  unsigned int num_pressure_outlets = 0;
  for (unsigned int enum_ind = 0; enum_ind < _outlet_boundaries.size(); ++enum_ind)
    if (_momentum_outlet_types[enum_ind] == "fixed-pressure" ||
        _momentum_outlet_types[enum_ind] == "fixed-pressure-zero-gradient")
      num_pressure_outlets += 1;

  checkSizeParam(_pressure_function.size(),
                 "pressure_function",
                 num_pressure_outlets,
                 "pressure outlet boundaries",
                 "'fixed-pressure/fixed-pressure-zero-gradient'");

  if (_compressibility == "incompressible" && _has_flow_equations)
    if (num_pressure_outlets == 0 && !(getParam<bool>("pin_pressure")))
      mooseError("The pressure must be fixed for an incompressible simulation! Try setting "
                 "pin_pressure or change the compressibility settings!");

  if (_has_energy_equation)
  {
    checkSizeFriendParams(_inlet_boundaries.size(),
                          _energy_inlet_types.size(),
                          "inlet_boundaries",
                          "energy_inlet_types",
                          "inlet boundaries");

    checkSizeFriendParams(_energy_inlet_types.size(),
                          _energy_inlet_function.size(),
                          "energy_inlet_types",
                          "energy_inlet_function",
                          "boundaries");

    checkSizeFriendParams(_wall_boundaries.size(),
                          _energy_wall_types.size(),
                          "wall_boundaries",
                          "energy_wall_types",
                          "wall boundaries");

    unsigned int num_fixed_energy_walls = 0;
    for (unsigned int enum_ind = 0; enum_ind < _energy_wall_types.size(); ++enum_ind)
      if (_energy_wall_types[enum_ind] == "fixed-temperature" ||
          _energy_wall_types[enum_ind] == "heatflux")
        num_fixed_energy_walls += 1;

    if (_wall_boundaries.size() > 0)
      checkSizeParam(_energy_wall_function.size(),
                     "energy_wall_function",
                     num_fixed_energy_walls,
                     "Dirichlet/Neumann conditions",
                     "'energy_wall_types'");
  }
  if (_has_scalar_equation)
  {
    if (_inlet_boundaries.size())
    {
      checkSizeFriendParams(_inlet_boundaries.size() * _passive_scalar_names.size(),
                            _passive_scalar_inlet_types.size(),
                            "inlet_boundaries",
                            "passive_scalar_inlet_types",
                            "inlet boundaries times number of transported scalars");
      checkSizeFriendParams(_passive_scalar_names.size(),
                            _passive_scalar_inlet_function.size(),
                            "passive_scalar_names",
                            "passive_scalar_inlet_function",
                            "names");
      for (const auto & passive_scalar_index : index_range(_passive_scalar_inlet_function))
        checkSizeFriendParams(_passive_scalar_inlet_function[passive_scalar_index].size(),
                              _inlet_boundaries.size(),
                              "passive_scalar_inlet_function index " +
                                  std::to_string(passive_scalar_index),
                              "inlet_boundaries",
                              "entries");
    }
  }
}

void
NSFVAction::checkAmbientConvectionParameterErrors()
{
  if (_has_energy_equation)
  {
    checkBlockwiseConsistency<MooseFunctorName>(
        "ambient_convection_blocks", {"ambient_convection_alpha", "ambient_temperature"});
  }
}

void
NSFVAction::checkFrictionParameterErrors()
{
  checkBlockwiseConsistency<std::vector<std::string>>("friction_blocks",
                                                      {"friction_types", "friction_coeffs"});
  for (unsigned int block_i = 0; block_i < _friction_types.size(); ++block_i)
    if (_friction_types[block_i].size() != _friction_coeffs[block_i].size())
    {
      std::string block_name = "";
      if (_friction_blocks.size())
        block_name = Moose::stringify(_friction_blocks[block_i]);
      else
        block_name = std::to_string(block_i);

      paramError("friction_coeffs",
                 "The number of friction coefficients for block(s): " + block_name +
                     " is not the same as the number of requested friction types!");
    }

  for (unsigned int block_i = 0; block_i < _friction_types.size(); ++block_i)
  {
    MultiMooseEnum ft("darcy forchheimer");
    ft.push_back(_friction_types[block_i]);

    for (const auto & name : ft.getNames())
    {
      unsigned int c = std::count(ft.begin(), ft.end(), name);
      if (c > 1)
      {
        std::string block_name = "";
        if (_friction_blocks.size())
          block_name = Moose::stringify(_friction_blocks[block_i]);
        else
          block_name = std::to_string(block_i);

        paramError("friction_types",
                   "The following keyword: " + name + " appeared more than once in block(s) " +
                       block_name + " of 'friction_types'.");
      }
    }
  }
}

void
NSFVAction::checkPassiveScalarParameterErrors()
{
  if (_passive_scalar_diffusivity.size())
    if (_passive_scalar_diffusivity.size() != _passive_scalar_names.size())
      paramError("passive_scalar_diffusivity",
                 "The number of diffusivities defined is not equal to the number of passive scalar "
                 "fields!");

  if (_passive_scalar_schmidt_number.size())
    if (_passive_scalar_schmidt_number.size() != _passive_scalar_names.size())
      paramError(
          "passive_scalar_schmidt_number",
          "The number of Schmidt numbers defined is not equal to the number of passive scalar "
          "fields!");

  if (_passive_scalar_source.size())
    if (_passive_scalar_source.size() != _passive_scalar_names.size())
      paramError("passive_scalar_source",
                 "The number of external sources defined is not equal to the number of passive "
                 "scalar fields!");

  if (_passive_scalar_coupled_source.size())
  {
    if (_passive_scalar_coupled_source.size() != _passive_scalar_names.size())
      paramError("passive_scalar_coupled_source",
                 "The number of coupled sources defined is not equal to the number of passive "
                 "scalar fields! Did you forget semicolons in the vector input?");

    if (_passive_scalar_coupled_source_coeff.size() != _passive_scalar_names.size())
      paramError("passive_scalar_coupled_source_coeff",
                 "The number of coupled sources coefficients defined is not equal to the "
                 "number of passive scalar fields! Did you forget semicolons in the vector input?");
    for (unsigned int i = 0; i < _passive_scalar_coupled_source.size(); i++)
    {
      if (_passive_scalar_coupled_source[i].size() !=
          _passive_scalar_coupled_source_coeff[i].size())
        paramError("passive_scalar_coupled_source_coeff",
                   "The number of coupled sources coefficients defined is not equal to the number "
                   "of coupled sources! Did you forget semicolons in the vector input?");
    }
  }
}

void
NSFVAction::checkDependentParameterError(const std::string main_parameter,
                                         const std::vector<std::string> dependent_parameters,
                                         const bool should_be_defined)
{
  for (const auto & param : dependent_parameters)
    if (_pars.isParamSetByUser(param) == !should_be_defined)
      paramError(param,
                 "This parameter should " + std::string(should_be_defined ? "" : "not") +
                     " be given by the user with the corresponding " + main_parameter +
                     " setting!");
}

void
NSFVAction::checkSizeFriendParams(const unsigned int param_vector_1_size,
                                  const unsigned int param_vector_2_size,
                                  const std::string & param_name_1,
                                  const std::string & param_name_2,
                                  const std::string & object_name_1) const
{
  // param 1 is the reference
  if (param_vector_1_size != param_vector_2_size)
    paramError(param_name_2,
               "Size (" + std::to_string(param_vector_2_size) +
                   ") is not the same as the "
                   "number of " +
                   object_name_1 + " in '" + param_name_1 + "' (size " +
                   std::to_string(param_vector_1_size) + ")");
}

void
NSFVAction::checkSizeParam(const unsigned int param_vector_size,
                           const std::string & param_name,
                           const unsigned int size_wanted,
                           const std::string & object_name,
                           const std::string & size_source_explanation) const
{
  if (param_vector_size != size_wanted)
    paramError(param_name,
               "Size (" + std::to_string(param_vector_size) +
                   ") is not the same as the number of " + object_name + " in " +
                   size_source_explanation + " (size " + std::to_string(size_wanted) + ")");
}

void
NSFVAction::checkRhieChowFunctorsDefined()
{
  if (!_problem->hasFunctor("ax", /*thread_id=*/0))
    paramError("add_flow_equations",
               "Rhie Chow coefficient ax must be provided for advection by auxiliary velocities");
  if (_dim >= 2 && !_problem->hasFunctor("ay", /*thread_id=*/0))
    paramError("add_flow_equations",
               "Rhie Chow coefficient ay must be provided for advection by auxiliary velocities");
  if (_dim == 3 && !_problem->hasFunctor("az", /*thread_id=*/0))
    paramError("add_flow_equations",
               "Rhie Chow coefficient az must be provided for advection by auxiliary velocities");
}
