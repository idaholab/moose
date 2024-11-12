//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVBase.h"
#include "NS.h"
#include "Action.h"
#include "INSFVMomentumAdvection.h"
#include "INSFVRhieChowInterpolator.h"

InputParameters
NSFVBase::commonNavierStokesFlowParams()
{
  InputParameters params = emptyInputParameters();
  MooseEnum comp_type("incompressible weakly-compressible", "incompressible");
  params.addParam<MooseEnum>(
      "compressibility", comp_type, "Compressibility constraint for the Navier-Stokes equations.");

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

  /**
   * Parameters used to define the boundaries of the domain.
   */

  params.addParam<std::vector<BoundaryName>>(
      "inlet_boundaries", std::vector<BoundaryName>(), "Names of inlet boundaries");
  params.addParam<std::vector<BoundaryName>>(
      "outlet_boundaries", std::vector<BoundaryName>(), "Names of outlet boundaries");
  params.addParam<std::vector<BoundaryName>>(
      "wall_boundaries", std::vector<BoundaryName>(), "Names of wall boundaries");
  return params;
}

InputParameters
NSFVBase::commonMomentumEquationParams()
{
  InputParameters params = emptyInputParameters();

  params.addParam<MooseFunctorName>(
      "dynamic_viscosity", NS::mu, "The name of the dynamic viscosity");
  params.addParam<MooseFunctorName>("density", NS::density, "The name of the density");

  // Dynamic pressure parameter
  // TODO: make default
  params.addParam<bool>("solve_for_dynamic_pressure",
                        false,
                        "Whether to solve for the dynamic pressure instead of the total pressure");

  // Pressure pin parameters
  params.addParam<bool>(
      "pin_pressure", false, "Switch to enable pressure shifting for incompressible simulations.");
  MooseEnum s_type("average point-value average-uo point-value-uo", "average-uo");
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

  params.addParam<RealVectorValue>(
      "gravity", RealVectorValue(0, 0, 0), "The gravitational acceleration vector.");

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
      "pin_pressure pinned_pressure_type pinned_pressure_point pinned_pressure_value ",
      "Incompressible flow pressure constraint");
  params.addParamNamesToGroup("ref_temperature boussinesq_approximation gravity",
                              "Gravity treatment");

  /**
   * Parameters controlling the friction terms in case of porous medium simulations.
   */
  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "friction_blocks",
      {},
      "The blocks where the friction factors are applied to emulate flow resistances.");

  params.addParam<std::vector<std::vector<std::string>>>(
      "friction_types", {}, "The types of friction forces for every block in 'friction_blocks'.");

  params.addParam<std::vector<std::vector<std::string>>>(
      "friction_coeffs",
      {},
      "The friction coefficients for every item in 'friction_types'. Note that if "
      "'porous_medium_treatment' is enabled, the coefficients already contain a velocity "
      "multiplier but they are not multiplied with density yet!");

  params.addParam<bool>(
      "standard_friction_formulation",
      true,
      "Flag to enable the standard friction formulation or its alternative, "
      "which is a simplified version (see user documentation for PINSFVMomentumFriction).");

  params.addParamNamesToGroup("friction_blocks friction_types friction_coeffs "
                              "standard_friction_formulation",
                              "Friction control");
  return params;
}

InputParameters
NSFVBase::commonMomentumBoundaryTypesParams()
{
  InputParameters params = emptyInputParameters();
  MultiMooseEnum mom_inlet_types("fixed-velocity flux-velocity flux-mass fixed-pressure");
  params.addParam<MultiMooseEnum>("momentum_inlet_types",
                                  mom_inlet_types,
                                  "Types of inlet boundaries for the momentum equation.");

  MultiMooseEnum mom_outlet_types("fixed-pressure zero-gradient fixed-pressure-zero-gradient");
  params.addParam<MultiMooseEnum>("momentum_outlet_types",
                                  mom_outlet_types,
                                  "Types of outlet boundaries for the momentum equation");
  params.addParam<std::vector<MooseFunctorName>>("pressure_function",
                                                 std::vector<MooseFunctorName>(),
                                                 "Functions for boundary pressures at outlets.");

  MultiMooseEnum mom_wall_types("symmetry noslip slip wallfunction");
  params.addParam<MultiMooseEnum>(
      "momentum_wall_types", mom_wall_types, "Types of wall boundaries for the momentum equation");

  return params;
}

InputParameters
NSFVBase::commonMomentumBoundaryFluxesParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "momentum_inlet_function",
      std::vector<std::vector<MooseFunctorName>>(),
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

  return params;
}

InputParameters
NSFVBase::commonFluidEnergyEquationParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<FunctionName>(
      "initial_temperature", "300", "The initial temperature, assumed constant everywhere");

  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "thermal_conductivity_blocks",
      {},
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

  params.addParam<std::vector<MooseFunctorName>>(
      "energy_inlet_function",
      std::vector<MooseFunctorName>(),
      "Functions for fixed-value boundaries in the energy equation.");

  MultiMooseEnum en_wall_types("fixed-temperature heatflux wallfunction");
  params.addParam<MultiMooseEnum>(
      "energy_wall_types", en_wall_types, "Types for the wall boundaries for the energy equation.");

  params.addParam<std::vector<MooseFunctorName>>(
      "energy_wall_function",
      std::vector<MooseFunctorName>(),
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
                              "ambient_temperature",
                              "Volumetric heat convection");
  params.addParamNamesToGroup("external_heat_source external_heat_source_coeff", "Heat source");
  params.addParamNamesToGroup("use_external_enthalpy_material", "Material properties");

  return params;
}

InputParameters
NSFVBase::commonScalarFieldAdvectionParams()
{
  InputParameters params = emptyInputParameters();
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

  MultiMooseEnum ps_inlet_types("fixed-value flux-mass flux-velocity");
  params.addParam<MultiMooseEnum>(
      "passive_scalar_inlet_types",
      ps_inlet_types,
      "Types for the inlet boundaries for the passive scalar equation.");

  params.addParamNamesToGroup("passive_scalar_names passive_scalar_diffusivity "
                              "passive_scalar_source passive_scalar_coupled_source "
                              "passive_scalar_coupled_source_coeff",
                              "Passive scalar control");
  return params;
}

InputParameters
NSFVBase::commonTurbulenceParams()
{
  InputParameters params = emptyInputParameters();

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

  params.addParam<MooseFunctorName>(
      "von_karman_const", 0.41, "Von Karman parameter for the mixing length model");
  params.addParam<MooseFunctorName>("von_karman_const_0", 0.09, "'Escudier' model parameter");
  params.addParam<MooseFunctorName>(
      "mixing_length_delta",
      1.0,
      "Tunable parameter related to the thickness of the boundary layer."
      "When it is not specified, Prandtl's original unbounded wall distance mixing length model is"
      "retrieved.");
  params.addRangeCheckedParam<Real>("turbulent_prandtl",
                                    1,
                                    "turbulent_prandtl > 0",
                                    "Turbulent Prandtl number for energy turbulent diffusion");
  params.addParam<std::vector<Real>>(
      "passive_scalar_schmidt_number",
      std::vector<Real>(),
      "Turbulent Schmidt numbers used for the passive scalar fields.");
  params.deprecateParam("passive_scalar_schmidt_number", "Sc_t", "01/01/2025");
  params.addParamNamesToGroup("mixing_length_walls mixing_length_aux_execute_on von_karman_const "
                              "von_karman_const_0 mixing_length_delta",
                              "Mixing length model");

  return params;
}

InputParameters
NSFVBase::validParams()
{
  InputParameters params = Action::validParams();

  /**
   * Add params relevant to the objects we may add
   */
  params += INSFVRhieChowInterpolator::uniqueParams();
  params += INSFVMomentumAdvection::uniqueParams();

  /**
   * General parameters used to set up the simulation.
   */
  params += NSFVBase::commonNavierStokesFlowParams();

  params.addParam<bool>(
      "porous_medium_treatment", false, "Whether to use porous medium kernels or not.");

  MooseEnum turbulence_type("mixing-length none", "none");
  params.addParam<MooseEnum>(
      "turbulence_handling",
      turbulence_type,
      "The way additional diffusivities are determined in the turbulent regime.");

  params.addParam<bool>("initialize_variables_from_mesh_file",
                        false,
                        "Determines if the variables that are added by the action are initialized "
                        "from the mesh file (only for Exodus format)");
  params.addParam<std::string>(
      "initial_from_file_timestep",
      "LATEST",
      "Gives the timestep (or \"LATEST\") for which to read a solution from a file "
      "for a given variable. (Default: LATEST)");

  params.addParam<bool>("add_flow_equations", true, "True to add mass and momentum equations");
  params.addParam<bool>("add_energy_equation", false, "True to add energy equation");
  params.addParam<bool>("add_scalar_equation", false, "True to add advected scalar(s) equation");

  params.addParamNamesToGroup("compressibility porous_medium_treatment turbulence_handling "
                              "add_flow_equations add_energy_equation add_scalar_equation ",
                              "General control");

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
                        false,
                        "If friction correction should be applied in the momentum equation.");

  params.addParam<Real>(
      "consistent_scaling",
      "Scaling parameter for the friction correction in the momentum equation (if requested).");

  params.addParamNamesToGroup("porosity porosity_smoothing_layers use_friction_correction "
                              "consistent_scaling porosity_interface_pressure_treatment",
                              "Porous medium treatment");

  /**
   * Parameters used to define the handling of the momentum-mass equations.
   */
  std::vector<FunctionName> default_initial_velocity = {"1e-15", "1e-15", "1e-15"};
  params.addParam<std::vector<FunctionName>>("initial_velocity",
                                             default_initial_velocity,
                                             "The initial velocity, assumed constant everywhere");

  params.addParam<FunctionName>(
      "initial_pressure", "1e5", "The initial pressure, assumed constant everywhere");

  params += NSFVBase::commonMomentumEquationParams();

  /**
   * Parameters describing the momentum equations boundary conditions
   */
  params += NSFVBase::commonMomentumBoundaryTypesParams();
  params += NSFVBase::commonMomentumBoundaryFluxesParams();

  /**
   * Parameters describing the fluid energy equation
   */
  params += NSFVBase::commonFluidEnergyEquationParams();

  /**
   * Parameters describing the handling of advected scalar fields
   */
  params += NSFVBase::commonScalarFieldAdvectionParams();

  // These parameters are not shared because the WCNSFVPhysics use functors
  params.addParam<std::vector<std::vector<std::string>>>(
      "passive_scalar_inlet_function",
      std::vector<std::vector<std::string>>(),
      "Functions for inlet boundaries in the passive scalar equations.");

  /**
   * Parameters describing the handling of turbulence
   */
  params += NSFVBase::commonTurbulenceParams();

  /**
   * Parameters allowing the control over numerical schemes for different terms in the
   * Navier-Stokes + energy equations.
   */

  MooseEnum adv_interpol_types(Moose::FV::interpolationMethods());
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
      "pressure_allow_expansion_on_bernoulli_faces",
      false,
      "Switch to enable the two-term extrapolation on porosity jump faces. "
      "WARNING: Depending on the mesh, enabling this parameter may lead to "
      "termination in parallel runs due to insufficient ghosting between "
      "processors. An example can be the presence of multiple porosity jumps separated by only "
      "one cell while using the Bernoulli pressure treatment. In such cases adjust the "
      "`ghost_layers` parameter. ");
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
      "pressure_allow_expansion_on_bernoulli_faces velocity_interpolation",
      "Numerical scheme");

  params.addParamNamesToGroup("momentum_scaling energy_scaling mass_scaling passive_scalar_scaling",
                              "Scaling");

  /**
   * Parameters controlling the ghosting/parallel execution
   */
  params.addRangeCheckedParam<unsigned short>(
      "ghost_layers",
      2,
      "ghost_layers > 0",
      "The number of geometric/algebraic/coupling layers to ghost.");

  params.addParamNamesToGroup("ghost_layers", "Parallel Execution Tuning");

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
