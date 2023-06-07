//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NS.h"
#include "FEProblem.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "NSFVUtils.h"

/**
 * Base class for setting up Navier-Stokes finite volume simulations
 */
template <class BaseType>
class NSFVBase : public BaseType
{
public:
  static InputParameters validParams();

  NSFVBase(const InputParameters & parameters);

protected:
  /// Type that we use in Actions for declaring coupling between the solutions
  /// of different physics components
  typedef std::vector<VariableName> CoupledName;

  /// Adds NS variables
  void addNSVariables();
  /// Adds NS initial conditions
  void addNSInitialConditions();
  /// Adds NS user objects
  void addNSUserObjects();
  /// Adds NS kernels
  void addNSKernels();
  /// Adds NS boundary conditions
  void addNSBoundaryConditions();
  /// Adds NS materials
  void addNSMaterials();
  /// Adds NS post-processors
  void addNSPostprocessors();
  /// Checks copy NS nodal variables
  void checkCopyNSNodalVariables();
  /// Copies NS nodal variables
  void copyNSNodalVariables();

  /// Functions for defining the variables depending on the compressibility option
  /// selected by the user
  void addINSVariables();

  /// Function which adds the RhieChow interpolator user objects for weakly and incompressible formulations
  void addRhieChowUserObjects();

  /// Function that add the initial conditions for the variables
  void addINSInitialConditions();

  /// Function adding kernels for the incompressible continuity equation
  void addINSMassKernels();

  /**
   * Functions adding kernels for the incompressible momentum equation
   * If the material properties are not constant, these can be used for
   * weakly-compressible simulations (except the Boussinesq kernel) as well.
   */
  void addINSMomentumTimeKernels();
  void addINSMomentumViscousDissipationKernels();
  void addINSMomentumMixingLengthKernels();
  void addINSMomentumAdvectionKernels();
  void addINSMomentumPressureKernels();
  void addINSMomentumGravityKernels();
  void addINSMomentumBoussinesqKernels();
  void addINSMomentumFrictionKernels();

  /**
   * Functions adding kernels for the incompressible energy equation
   * If the material properties are not constant, some of these can be used for
   * weakly-compressible simulations as well.
   */
  void addINSEnergyTimeKernels();
  void addINSEnergyHeatConductionKernels();
  void addINSEnergyAdvectionKernels();
  void addINSEnergyAmbientConvection();
  void addINSEnergyExternalHeatSource();

  /**
   * Functions adding kernels for scalar transport equations in a weakly or in-compressible
   * fluid.
   */
  void addScalarTimeKernels();
  void addScalarAdvectionKernels();
  void addScalarDiffusionKernels();
  void addScalarMixingLengthKernels();
  void addScalarSourceKernels();
  void addScalarCoupledSourceKernels();

  /// Functions adding boundary conditions for the incompressible simulation.
  /// These are used for weakly-compressible simulations as well.
  void addINSInletBC();
  void addINSOutletBC();
  void addINSWallBC();

  void addINSEnergyInletBC();
  void addINSEnergyWallBC();

  void addScalarInletBC();

  /// Functions which add time kernels for transient, weakly-compressible simulations.
  void addWCNSMassTimeKernels();
  void addWCNSMomentumTimeKernels();
  void addWCNSEnergyTimeKernels();

  /// Add weakly compressible mixing length kernels to the energy equation
  /// in case of turbulent flows
  void addWCNSEnergyMixingLengthKernels();
  /// Add material to define an enthalpy functor material property for incompressible simulations
  void addEnthalpyMaterial();
  /// Add material to define the local speed in porous medium flows
  void addPorousMediumSpeedMaterial();
  /// Add mixing length material for turbulence handling
  void addMixingLengthMaterial();

  /// Add boundary postprocessors for flux BCs
  void addBoundaryPostprocessors();

  /**
   * Gets the MOOSE object parameters to use to add the relationship manager to
   * extend the number of ghosted layers if necessary.
   */
  InputParameters getGhostParametersForRM();

  /// Process the mesh data and convert block names to block IDs
  void processMesh();
  /// Assign the necessary blocks to the input parameters
  void assignBlocks(InputParameters & params, const std::vector<SubdomainName> & blocks);
  /// Process the supplied variable names and if they are not available, create them
  void processVariables();
  /// Checks if the input variables are restricted to the same blocks as the action
  void checkVariableBlockConsistency(const std::string & var_name);
  /// Process thermal conductivity (multiple functor input options are available).
  /// Return true if we have vector thermal conductivity and false if scalar
  bool processThermalConductivity();
  /// Check for general user errors in the parameters
  void checkGeneralControlErrors();
  /// Check errors regarding the user defined boundary treatments
  void checkICParameterErrors();
  /// Check errors regarding the user defined boundary treatments
  void checkBoundaryParameterErrors();
  /// Check errors regarding the user defined ambient convection parameters
  void checkAmbientConvectionParameterErrors();
  /// Check errors regarding the friction parameters
  void checkFrictionParameterErrors();
  /// Check errors regarding the passive scalars
  void checkPassiveScalarParameterErrors();
  /// Check if the user commited errors during the definition of block-wise
  /// parameters
  template <typename T>
  void checkBlockwiseConsistency(const std::string block_param_name,
                                 const std::vector<std::string> parameter_names);
  /// Throws an error if any of the parameters are defined from a vector while the
  /// the corresponding main parameter is disabled
  void checkDependentParameterError(const std::string & main_parameter,
                                    const std::vector<std::string> & dependent_parameters,
                                    const bool should_be_defined = false);

  /// Checks that sufficient Rhie Chow coefficients have been defined for the given dimension, used
  /// for scalar or temperature advection by auxiliary variables
  void checkRhieChowFunctorsDefined();
  /// Check that two parameters are the same size
  void checkSizeFriendParams(const unsigned int param_vector_1_size,
                             const unsigned int param_vector_2_size,
                             const std::string & param_name_1,
                             const std::string & param_name_2,
                             const std::string & object_name_1) const;
  /// Check that a parameter is of the expected size
  void checkSizeParam(const unsigned int param_vector_size,
                      const std::string & param_name,
                      const unsigned int size_wanted,
                      const std::string & object_name,
                      const std::string & size_source_explanation) const;

  /// Returns thet list of block names
  virtual std::vector<SubdomainName> getBlocks() const = 0;

  /// Returns the factory
  virtual Factory & getFactory() = 0;

  /// Returns the problem
  virtual FEProblemBase & getProblem() = 0;

  /// Returns the mesh
  virtual const MooseMesh & getMesh() const = 0;

  /**
   * Adds a nonlinear (solution) variable
   *
   * @param[in] var_type   Variable type
   * @param[in] var_name   Variable name
   * @param[in] params     Variable parameters
   */
  virtual void addNSNonlinearVariable(const std::string & var_type,
                                      const std::string & var_name,
                                      InputParameters & params) = 0;

  /**
   * Adds an aux variable
   *
   * @param[in] var_type   Variable type
   * @param[in] var_name   Variable name
   * @param[in] params     Variable parameters
   */
  virtual void addNSAuxVariable(const std::string & var_type,
                                const std::string & var_name,
                                InputParameters & params) = 0;

  /**
   * Adds an initial condition
   *
   * @param[in] type    IC type
   * @param[in] name    IC name
   * @param[in] params  IC parameters
   */
  virtual void addNSInitialCondition(const std::string & type,
                                     const std::string & name,
                                     InputParameters & params) = 0;

  /// Returns the naming prefix
  virtual std::string prefix() const = 0;

  /// Subdomains Navier-Stokes equation is defined on
  std::vector<SubdomainName> _blocks;

  /// Mesh dimension
  unsigned int _dim;

  /// Compressibility type, can be compressible, incompressible
  /// or weakly-compressible
  const MooseEnum _compressibility;
  /// Switch that can be used to or not pressure and mass equations
  /// for incompressible/weakly compressible simulations.
  const bool _has_flow_equations;
  /// Switch that can be used to create an integrated energy equation for
  /// incompressible/weakly compressible simulations.
  const bool _has_energy_equation;
  /// Switch that can be used to create an integrated energy equation for
  /// incompressible/weakly compressible simulations.
  const bool _has_scalar_equation;
  /// Switch to use to enable the Boussinesq approximation for incompressible
  /// fluid simulations
  const bool _boussinesq_approximation;
  /// Turbulent diffusivity handling type (mixing-length, etc.)
  const MooseEnum _turbulence_handling;

  /// Switch to show if porous medium treatment is requested or not
  const bool _porous_medium_treatment;
  /// The name of the functor for the porosity field
  const MooseFunctorName _porosity_name;
  /// The name of the functor for the smoothed porosity field
  const MooseFunctorName _flow_porosity_functor_name;
  /// Switch to enable friction correction for the porous medium momentum
  /// equations
  const bool _use_friction_correction;

  /// Velocity names in case they are defined externally. For example in a situation when
  /// we need to restart a simulation
  const std::vector<std::string> _velocity_name;
  /// Pressure name in case we want to define it externally
  const NonlinearVariableName _pressure_name;
  /// Fluid temperature name in case we want to define it externally
  const NonlinearVariableName _fluid_temperature_name;

  /// Boundaries with a flow inlet specified on them
  const std::vector<BoundaryName> _inlet_boundaries;
  /// Boundaries with a flow outlet specified on them
  const std::vector<BoundaryName> _outlet_boundaries;
  /// Boundaries which define a wall (slip/noslip/etc.)
  const std::vector<BoundaryName> _wall_boundaries;

  /// Velocity inlet types (fixed-velocity/mass-flow/momentum-inflow)
  const MultiMooseEnum _momentum_inlet_types;
  /// Momentum/mass inlet flux postprocessors for potential coupling between applications
  const std::vector<PostprocessorName> _flux_inlet_pps;
  /// Momentum/mass inlet flux directions for potential coupling between applications
  const std::vector<Point> _flux_inlet_directions;
  /// Velocity function names at velocity inlet boundaries
  const std::vector<std::vector<FunctionName>> _momentum_inlet_function;
  /// Velocity outlet types (pressure/mass-outflow/momentum-outflow)
  const MultiMooseEnum _momentum_outlet_types;
  /// Velocity wall types (symmetry/noslip/slip/wallfunction)
  const MultiMooseEnum _momentum_wall_types;

  /// Energy intlet types (fixed-velocity/mass-flow/momentum-inflow)
  const MultiMooseEnum _energy_inlet_types;
  /// Energy function names at inlet boundaries
  const std::vector<std::string> _energy_inlet_function;
  /// Energy wall types (symmetry/heatflux/fixed-temperature)
  const MultiMooseEnum _energy_wall_types;
  /// Energy function names at wall boundaries
  const std::vector<FunctionName> _energy_wall_function;

  /// Pressure function names at pressure boundaries
  const std::vector<FunctionName> _pressure_function;

  /// Subdomains where we want to have ambient convection
  const std::vector<std::vector<SubdomainName>> _ambient_convection_blocks;
  /// The heat exchange coefficients for ambient convection
  const std::vector<MooseFunctorName> _ambient_convection_alpha;
  /// The ambient temperature
  const std::vector<MooseFunctorName> _ambient_temperature;

  /// Subdomains where we want to have volumetric friction
  const std::vector<std::vector<SubdomainName>> _friction_blocks;
  /// The friction correlation types used for each block
  const std::vector<std::vector<std::string>> _friction_types;
  /// The coefficients used for each item if friction type
  const std::vector<std::vector<std::string>> _friction_coeffs;

  /// Name of the density material property
  const MooseFunctorName _density_name;
  /// Name of the dynamic viscosity material property
  const MooseFunctorName _dynamic_viscosity_name;
  /// Name of the specific heat material property
  const MooseFunctorName _specific_heat_name;
  /// Subdomains where we want to have different thermal conduction
  const std::vector<std::vector<SubdomainName>> _thermal_conductivity_blocks;
  /// Name of the thermal conductivity functor for each block-group
  const std::vector<MooseFunctorName> _thermal_conductivity_name;
  /// Name of the thermal expansion material property
  const MooseFunctorName _thermal_expansion_name;

  /// List of the names of the scalar variables which are transported using this
  /// action
  const std::vector<NonlinearVariableName> _passive_scalar_names;
  /// Initial values for the passive scalar fields
  const std::vector<Real> _initial_scalar_variable;
  /// Passive scalar diffusivities
  const std::vector<MooseFunctorName> _passive_scalar_diffusivity;
  /// Passive scalar Schmidt numbers
  const std::vector<Real> _passive_scalar_schmidt_number;
  /// Passive scalar external source terms
  const std::vector<MooseFunctorName> _passive_scalar_source;
  /// Passive scalar coupled source terms
  const std::vector<std::vector<MooseFunctorName>> _passive_scalar_coupled_source;
  /// Passive scalar coupled source term coeffs
  const std::vector<std::vector<Real>> _passive_scalar_coupled_source_coeff;
  /// Passive scalar inlet types (fixed-value/mass-flow)
  const MultiMooseEnum _passive_scalar_inlet_types;
  /// Passive scalar function names at inlet boundaries
  const std::vector<std::vector<std::string>> _passive_scalar_inlet_function;

  /// The type of the advected quantity interpolation method for continuity equation
  const MooseEnum _mass_advection_interpolation;
  /// The type of the advected quantity interpolation method for momentum/velocity
  const MooseEnum _momentum_advection_interpolation;
  /// The type of the advected quantity interpolation method for energy/temperature
  const MooseEnum _energy_advection_interpolation;
  /// The type of the advected quantity interpolation method for passive scalars
  const MooseEnum _passive_scalar_advection_interpolation;

  /// The type of the pressure interpolation method
  const MooseEnum _pressure_face_interpolation;
  /// The type of the face interpolation method for the velocity/momentum
  const MooseEnum _momentum_face_interpolation;
  /// The type of the face interpolation method for the temperature/energy
  const MooseEnum _energy_face_interpolation;
  /// The type of the face interpolation method for the passive scalar fields
  const MooseEnum _passive_scalar_face_interpolation;

  /// The type of velocity interpolation to perform
  const MooseEnum _velocity_interpolation;

  /// If a two-term Taylor expansion is needed for the determination of the boundary values
  /// of the pressure
  const bool _pressure_two_term_bc_expansion;
  /// Switch to enable the two-term extrapolation on porosity jump faces.
  const bool _pressure_allow_expansion_on_bernoulli_faces;
  /// If a two-term Taylor expansion is needed for the determination of the boundary values
  /// of the velocity/momentum
  const bool _momentum_two_term_bc_expansion;
  /// If a two-term Taylor expansion is needed for the determination of the boundary values
  /// of the temperature/energy
  const bool _energy_two_term_bc_expansion;
  /// If a two-term Taylor expansion is needed for the determination of the boundary values
  /// of the passive scalar fields
  const bool _passive_scalar_two_term_bc_expansion;

  /// The scaling factor for the mass equation variable (for incompressible simulations this is pressure scaling)
  const Real _mass_scaling;
  /// The scaling factor for the momentum variables
  const Real _momentum_scaling;
  /// The scaling factor for the energy variable (for incompressible simulations, temperature)
  const Real _energy_scaling;
  /// The scaling factor for the passive scalar variables
  const Real _passive_scalar_scaling;

  /// List to show which advected scalar field variable needs to be created within
  /// this action
  std::vector<bool> _create_scalar_variable;
  /// Boolean showing if the velocity is created within the action or outside the action
  bool _create_velocity;
  /// Boolean showing if the pressure is created in the action or not
  bool _create_pressure;
  /// Boolean showing if the fluid tempreture is created in the action or not
  bool _create_fluid_temperature;
  /// Boolean showing if the user wants to disable the generation of the enthalpy
  /// material within this action
  bool _use_external_enthalpy_material;

  using BaseType::_app;
  using BaseType::paramError;
  using BaseType::parameters;
};

template <class BaseType>
InputParameters
NSFVBase<BaseType>::validParams()
{
  InputParameters params = BaseType::validParams();

  /**
   * General parameters used to set up the simulation.
   */

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

template <class BaseType>
NSFVBase<BaseType>::NSFVBase(const InputParameters & parameters)
  : BaseType(parameters),
    _compressibility(parameters.get<MooseEnum>("compressibility")),
    _has_flow_equations(parameters.get<bool>("add_flow_equations")),
    _has_energy_equation(parameters.get<bool>("add_energy_equation")),
    _has_scalar_equation(parameters.get<bool>("add_scalar_equation")),
    _boussinesq_approximation(parameters.get<bool>("boussinesq_approximation")),
    _turbulence_handling(parameters.get<MooseEnum>("turbulence_handling")),
    _porous_medium_treatment(parameters.get<bool>("porous_medium_treatment")),
    _porosity_name(parameters.get<MooseFunctorName>("porosity")),
    _flow_porosity_functor_name(parameters.isParamValid("porosity_smoothing_layers") &&
                                        parameters.get<unsigned short>("porosity_smoothing_layers")
                                    ? NS::smoothed_porosity
                                    : _porosity_name),
    _use_friction_correction(parameters.isParamValid("use_friction_correction")
                                 ? parameters.get<bool>("use_friction_correction")
                                 : false),
    _velocity_name(
        parameters.isParamValid("velocity_variable")
            ? parameters.get<std::vector<std::string>>("velocity_variable")
            : (_porous_medium_treatment
                   ? std::vector<std::string>(NS::superficial_velocity_vector,
                                              NS::superficial_velocity_vector + 3)
                   : std::vector<std::string>(NS::velocity_vector, NS::velocity_vector + 3))),
    _pressure_name(parameters.isParamValid("pressure_variable")
                       ? parameters.get<NonlinearVariableName>("pressure_variable")
                       : NS::pressure),
    _fluid_temperature_name(
        parameters.isParamValid("fluid_temperature_variable")
            ? parameters.get<NonlinearVariableName>("fluid_temperature_variable")
            : NS::T_fluid),
    _inlet_boundaries(parameters.get<std::vector<BoundaryName>>("inlet_boundaries")),
    _outlet_boundaries(parameters.get<std::vector<BoundaryName>>("outlet_boundaries")),
    _wall_boundaries(parameters.get<std::vector<BoundaryName>>("wall_boundaries")),
    _momentum_inlet_types(parameters.get<MultiMooseEnum>("momentum_inlet_types")),
    _flux_inlet_pps(parameters.get<std::vector<PostprocessorName>>("flux_inlet_pps")),
    _flux_inlet_directions(parameters.get<std::vector<Point>>("flux_inlet_directions")),
    _momentum_inlet_function(
        parameters.get<std::vector<std::vector<FunctionName>>>("momentum_inlet_function")),
    _momentum_outlet_types(parameters.get<MultiMooseEnum>("momentum_outlet_types")),
    _momentum_wall_types(parameters.get<MultiMooseEnum>("momentum_wall_types")),
    _energy_inlet_types(parameters.get<MultiMooseEnum>("energy_inlet_types")),
    _energy_inlet_function(parameters.get<std::vector<std::string>>("energy_inlet_function")),
    _energy_wall_types(parameters.get<MultiMooseEnum>("energy_wall_types")),
    _energy_wall_function(parameters.get<std::vector<FunctionName>>("energy_wall_function")),
    _pressure_function(parameters.get<std::vector<FunctionName>>("pressure_function")),
    _ambient_convection_blocks(
        parameters.get<std::vector<std::vector<SubdomainName>>>("ambient_convection_blocks")),
    _ambient_convection_alpha(
        parameters.get<std::vector<MooseFunctorName>>("ambient_convection_alpha")),
    _ambient_temperature(parameters.get<std::vector<MooseFunctorName>>("ambient_temperature")),
    _friction_blocks(
        parameters.isParamValid("friction_blocks")
            ? parameters.get<std::vector<std::vector<SubdomainName>>>("friction_blocks")
            : std::vector<std::vector<SubdomainName>>()),
    _friction_types(parameters.isParamValid("friction_types")
                        ? parameters.get<std::vector<std::vector<std::string>>>("friction_types")
                        : std::vector<std::vector<std::string>>()),
    _friction_coeffs(parameters.isParamValid("friction_coeffs")
                         ? parameters.get<std::vector<std::vector<std::string>>>("friction_coeffs")
                         : std::vector<std::vector<std::string>>()),
    _density_name(parameters.get<MooseFunctorName>("density")),
    _dynamic_viscosity_name(parameters.get<MooseFunctorName>("dynamic_viscosity")),
    _specific_heat_name(parameters.get<MooseFunctorName>("specific_heat")),
    _thermal_conductivity_blocks(
        parameters.isParamValid("thermal_conductivity_blocks")
            ? parameters.get<std::vector<std::vector<SubdomainName>>>("thermal_conductivity_blocks")
            : std::vector<std::vector<SubdomainName>>()),
    _thermal_conductivity_name(
        parameters.get<std::vector<MooseFunctorName>>("thermal_conductivity")),
    _thermal_expansion_name(parameters.get<MooseFunctorName>("thermal_expansion")),
    _passive_scalar_names(
        parameters.get<std::vector<NonlinearVariableName>>("passive_scalar_names")),
    _passive_scalar_diffusivity(
        parameters.get<std::vector<MooseFunctorName>>("passive_scalar_diffusivity")),
    _passive_scalar_schmidt_number(
        parameters.get<std::vector<Real>>("passive_scalar_schmidt_number")),
    _passive_scalar_source(parameters.get<std::vector<MooseFunctorName>>("passive_scalar_source")),
    _passive_scalar_coupled_source(parameters.get<std::vector<std::vector<MooseFunctorName>>>(
        "passive_scalar_coupled_source")),
    _passive_scalar_coupled_source_coeff(
        parameters.get<std::vector<std::vector<Real>>>("passive_scalar_coupled_source_coeff")),
    _passive_scalar_inlet_types(parameters.get<MultiMooseEnum>("passive_scalar_inlet_types")),
    _passive_scalar_inlet_function(
        parameters.get<std::vector<std::vector<std::string>>>("passive_scalar_inlet_function")),
    _mass_advection_interpolation(parameters.get<MooseEnum>("mass_advection_interpolation")),
    _momentum_advection_interpolation(
        parameters.get<MooseEnum>("momentum_advection_interpolation")),
    _energy_advection_interpolation(parameters.get<MooseEnum>("energy_advection_interpolation")),
    _passive_scalar_advection_interpolation(
        parameters.get<MooseEnum>("passive_scalar_advection_interpolation")),
    _pressure_face_interpolation(parameters.get<MooseEnum>("pressure_face_interpolation")),
    _momentum_face_interpolation(parameters.get<MooseEnum>("momentum_face_interpolation")),
    _energy_face_interpolation(parameters.get<MooseEnum>("energy_face_interpolation")),
    _passive_scalar_face_interpolation(
        parameters.get<MooseEnum>("passive_scalar_face_interpolation")),
    _velocity_interpolation(parameters.get<MooseEnum>("velocity_interpolation")),
    _pressure_two_term_bc_expansion(parameters.get<bool>("pressure_two_term_bc_expansion")),
    _pressure_allow_expansion_on_bernoulli_faces(
        parameters.get<bool>("pressure_allow_expansion_on_bernoulli_faces")),
    _momentum_two_term_bc_expansion(parameters.get<bool>("momentum_two_term_bc_expansion")),
    _energy_two_term_bc_expansion(parameters.get<bool>("energy_two_term_bc_expansion")),
    _passive_scalar_two_term_bc_expansion(
        parameters.get<bool>("passive_scalar_two_term_bc_expansion")),
    _mass_scaling(parameters.get<Real>("mass_scaling")),
    _momentum_scaling(parameters.get<Real>("momentum_scaling")),
    _energy_scaling(parameters.get<Real>("energy_scaling")),
    _passive_scalar_scaling(parameters.get<Real>("passive_scalar_scaling")),
    _create_velocity(!parameters.isParamValid("velocity_variable")),
    _create_pressure(!parameters.isParamValid("pressure_variable")),
    _create_fluid_temperature(!parameters.isParamValid("fluid_temperature_variable")),
    _use_external_enthalpy_material(parameters.get<bool>("use_external_enthalpy_material"))
{
  // Running the general checks, the rest are run after we already know some
  // geometry-related parameters.
  checkGeneralControlErrors();
}

template <class BaseType>
void
NSFVBase<BaseType>::addNSVariables()
{
  // Determine which variables to add
  processVariables();

  if (_compressibility == "weakly-compressible" || _compressibility == "incompressible")
    addINSVariables();
}

template <class BaseType>
void
NSFVBase<BaseType>::addNSInitialConditions()
{
  // Check initial condition related user input errors
  checkICParameterErrors();

  if (_compressibility == "incompressible" || _compressibility == "weakly-compressible")
    addINSInitialConditions();
}

template <class BaseType>
void
NSFVBase<BaseType>::addNSUserObjects()
{
  if (_compressibility == "incompressible" || _compressibility == "weakly-compressible")
    addRhieChowUserObjects();
}

template <class BaseType>
void
NSFVBase<BaseType>::addNSKernels()
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
      if (getProblem().isTransient())
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
      if (getProblem().isTransient())
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
      if (parameters().isParamValid("external_heat_source"))
        addINSEnergyExternalHeatSource();

      if (_turbulence_handling == "mixing-length")
        addWCNSEnergyMixingLengthKernels();
    }
    if (_has_scalar_equation)
    {
      if (getProblem().isTransient())
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

template <class BaseType>
void
NSFVBase<BaseType>::addNSBoundaryConditions()
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

template <class BaseType>
void
NSFVBase<BaseType>::addNSMaterials()
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

template <class BaseType>
void
NSFVBase<BaseType>::addNSPostprocessors()
{
  // Check if the user defined the boundary conditions in a sensible way
  checkBoundaryParameterErrors();

  addBoundaryPostprocessors();
}

template <class BaseType>
void
NSFVBase<BaseType>::checkCopyNSNodalVariables()
{
  if (parameters().template get<bool>("initialize_variables_from_mesh_file"))
    _app.setExodusFileRestart(true);
}

template <class BaseType>
void
NSFVBase<BaseType>::copyNSNodalVariables()
{
  if (parameters().template get<bool>("initialize_variables_from_mesh_file"))
  {
    SystemBase & system = getProblem().getNonlinearSystemBase();

    if (_create_pressure)
      system.addVariableToCopy(
          _pressure_name,
          _pressure_name,
          parameters().template get<std::string>("initial_from_file_timestep"));

    if (_create_velocity)
      for (unsigned int d = 0; d < _dim; ++d)
        system.addVariableToCopy(
            _velocity_name[d],
            _velocity_name[d],
            parameters().template get<std::string>("initial_from_file_timestep"));

    if (parameters().template get<bool>("pin_pressure"))
      system.addVariableToCopy(
          "lambda", "lambda", parameters().template get<std::string>("initial_from_file_timestep"));

    if (_turbulence_handling == "mixing-length")
      getProblem().getAuxiliarySystem().addVariableToCopy(
          NS::mixing_length,
          NS::mixing_length,
          parameters().template get<std::string>("initial_from_file_timestep"));

    if (_has_energy_equation && _create_fluid_temperature)
      system.addVariableToCopy(
          _fluid_temperature_name,
          _fluid_temperature_name,
          parameters().template get<std::string>("initial_from_file_timestep"));

    if (_has_scalar_equation)
      for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
      {
        bool create_me = true;
        if (_create_scalar_variable.size())
          if (!_create_scalar_variable[name_i])
            create_me = false;
        if (create_me)
          system.addVariableToCopy(
              _passive_scalar_names[name_i],
              _passive_scalar_names[name_i],
              parameters().template get<std::string>("initial_from_file_timestep"));
      }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSVariables()
{
  // Add velocity variable
  if (_create_velocity)
  {
    std::string variable_type = "INSFVVelocityVariable";
    if (_porous_medium_treatment)
      variable_type = "PINSFVSuperficialVelocityVariable";

    auto params = getFactory().getValidParams(variable_type);
    assignBlocks(params, _blocks);
    params.template set<std::vector<Real>>("scaling") = {_momentum_scaling};
    params.template set<MooseEnum>("face_interp_method") = _momentum_face_interpolation;
    params.template set<bool>("two_term_boundary_expansion") = _momentum_two_term_bc_expansion;

    for (unsigned int d = 0; d < _dim; ++d)
      addNSNonlinearVariable(variable_type, _velocity_name[d], params);
  }

  // Add pressure variable
  if (_create_pressure)
  {
    const bool using_pinsfv_pressure_var =
        _porous_medium_treatment && parameters().template get<MooseEnum>(
                                        "porosity_interface_pressure_treatment") != "automatic";
    const auto pressure_type =
        using_pinsfv_pressure_var ? "BernoulliPressureVariable" : "INSFVPressureVariable";
    auto params = getFactory().getValidParams(pressure_type);
    assignBlocks(params, _blocks);
    params.template set<std::vector<Real>>("scaling") = {_mass_scaling};
    params.template set<MooseEnum>("face_interp_method") = _pressure_face_interpolation;
    params.template set<bool>("two_term_boundary_expansion") = _pressure_two_term_bc_expansion;
    if (using_pinsfv_pressure_var)
    {
      params.template set<MooseFunctorName>("u") = _velocity_name[0];
      if (_dim >= 2)
        params.template set<MooseFunctorName>("v") = _velocity_name[1];
      if (_dim == 3)
        params.template set<MooseFunctorName>("w") = _velocity_name[2];
      params.template set<MooseFunctorName>(NS::porosity) = _porosity_name;
      params.template set<MooseFunctorName>(NS::density) = _density_name;
      params.template set<bool>("allow_two_term_expansion_on_bernoulli_faces") =
          _pressure_allow_expansion_on_bernoulli_faces;
    }

    addNSNonlinearVariable(pressure_type, _pressure_name, params);
  }

  // Add lagrange multiplier for pinning pressure, if needed
  if (parameters().template get<bool>("pin_pressure"))
  {
    auto lm_params = getFactory().getValidParams("MooseVariableScalar");
    lm_params.template set<MooseEnum>("family") = "scalar";
    lm_params.template set<MooseEnum>("order") = "first";

    addNSNonlinearVariable("MooseVariableScalar", "lambda", lm_params);
  }

  // Add turbulence-related variables
  if (_turbulence_handling == "mixing-length")
  {
    auto params = getFactory().getValidParams("MooseVariableFVReal");
    assignBlocks(params, _blocks);
    params.template set<bool>("two_term_boundary_expansion") =
        parameters().template get<bool>("mixing_length_two_term_bc_expansion");

    addNSAuxVariable("MooseVariableFVReal", NS::mixing_length, params);
  }

  // Add energy variables if needed
  if (_has_energy_equation)
  {
    if (_create_fluid_temperature)
    {
      auto params = getFactory().getValidParams("INSFVEnergyVariable");
      assignBlocks(params, _blocks);
      params.template set<std::vector<Real>>("scaling") = {_energy_scaling};
      params.template set<MooseEnum>("face_interp_method") = _energy_face_interpolation;
      params.template set<bool>("two_term_boundary_expansion") = _energy_two_term_bc_expansion;

      addNSNonlinearVariable("INSFVEnergyVariable", _fluid_temperature_name, params);
    }
  }

  // Add passive scalar variables if needed
  if (_has_scalar_equation)
  {
    auto params = getFactory().getValidParams("INSFVScalarFieldVariable");
    assignBlocks(params, _blocks);
    params.template set<std::vector<Real>>("scaling") = {_passive_scalar_scaling};
    params.template set<MooseEnum>("face_interp_method") = _passive_scalar_face_interpolation;
    params.template set<bool>("two_term_boundary_expansion") =
        _passive_scalar_two_term_bc_expansion;

    for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
    {
      bool create_me = true;
      if (_create_scalar_variable.size())
        if (!_create_scalar_variable[name_i])
          create_me = false;

      if (create_me)
        addNSNonlinearVariable("INSFVScalarFieldVariable", _passive_scalar_names[name_i], params);
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addRhieChowUserObjects()
{
  const std::string u_names[3] = {"u", "v", "w"};
  if (_porous_medium_treatment)
  {
    auto params = getFactory().getValidParams("PINSFVRhieChowInterpolator");
    assignBlocks(params, _blocks);
    for (unsigned int d = 0; d < _dim; ++d)
      params.template set<VariableName>(u_names[d]) = _velocity_name[d];

    params.template set<VariableName>("pressure") = _pressure_name;
    params.template set<MooseFunctorName>(NS::porosity) = _porosity_name;
    unsigned short smoothing_layers =
        parameters().isParamValid("porosity_smoothing_layers")
            ? parameters().template get<unsigned short>("porosity_smoothing_layers")
            : 0;
    params.template set<unsigned short>("smoothing_layers") = smoothing_layers;
    getProblem().addUserObject(
        "PINSFVRhieChowInterpolator", prefix() + "pins_rhie_chow_interpolator", params);
  }
  else
  {
    auto params = getFactory().getValidParams("INSFVRhieChowInterpolator");
    assignBlocks(params, _blocks);
    for (unsigned int d = 0; d < _dim; ++d)
      params.template set<VariableName>(u_names[d]) = _velocity_name[d];

    params.template set<VariableName>("pressure") = _pressure_name;
    // Set RhieChow coefficients
    if (!_has_flow_equations)
    {
      checkRhieChowFunctorsDefined();
      params.template set<MooseFunctorName>("a_u") = "ax";
      params.template set<MooseFunctorName>("a_v") = "ay";
      params.template set<MooseFunctorName>("a_w") = "az";
    }

    getProblem().addUserObject(
        "INSFVRhieChowInterpolator", prefix() + "ins_rhie_chow_interpolator", params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSInitialConditions()
{
  // do not set initial conditions if we load from file
  if (parameters().template get<bool>("initialize_variables_from_mesh_file"))
    return;

  InputParameters params = getFactory().getValidParams("FunctionIC");
  assignBlocks(params, _blocks);
  auto vvalue = parameters().template get<std::vector<FunctionName>>("initial_velocity");

  if (_create_velocity &&
      (!_app.isRestarting() || parameters().isParamSetByUser("initial_velocity")))
    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.template set<VariableName>("variable") = _velocity_name[d];
      params.template set<FunctionName>("function") = vvalue[d];

      addNSInitialCondition("FunctionIC", prefix() + _velocity_name[d] + "_ic", params);
    }

  if (_create_pressure &&
      (!_app.isRestarting() || parameters().isParamSetByUser("initial_pressure")))
  {
    params.template set<VariableName>("variable") = _pressure_name;
    params.template set<FunctionName>("function") =
        parameters().template get<FunctionName>("initial_pressure");

    addNSInitialCondition("FunctionIC", prefix() + _pressure_name + "_ic", params);
  }

  if (_has_energy_equation && _create_fluid_temperature &&
      (!_app.isRestarting() || parameters().isParamSetByUser("initial_temperature")))
  {
    params.template set<VariableName>("variable") = _fluid_temperature_name;
    params.template set<FunctionName>("function") =
        parameters().template get<FunctionName>("initial_temperature");

    addNSInitialCondition("FunctionIC", prefix() + _fluid_temperature_name + "_ic", params);
  }

  if (_has_scalar_equation &&
      (!_app.isRestarting() || parameters().isParamSetByUser("initial_scalar_variables")))
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
        params.template set<VariableName>("variable") = _passive_scalar_names[name_i];
        if (parameters().isParamValid("initial_scalar_variables"))
          params.template set<FunctionName>("function") =
              parameters().template get<std::vector<FunctionName>>(
                  "initial_scalar_variables")[ic_counter];
        else
          params.template set<FunctionName>("function") = "0.0";

        addNSInitialCondition(
            "FunctionIC", prefix() + _passive_scalar_names[name_i] + "_ic", params);
        ic_counter += 1;
      }
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSMomentumTimeKernels()
{
  std::string kernel_type = "INSFVMomentumTimeDerivative";
  std::string kernel_name = prefix() + "ins_momentum_time_";
  std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumTimeDerivative";
    kernel_name = prefix() + "pins_momentum_time_";
    rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.template set<MooseFunctorName>(NS::density) = _density_name;
  params.template set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
    params.template set<MooseEnum>("momentum_component") = NS::directions[d];

    getProblem().addFVKernel(kernel_type, kernel_name + _velocity_name[d], params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSEnergyTimeKernels()
{
  std::string kernel_type = "INSFVEnergyTimeDerivative";
  std::string kernel_name = prefix() + "ins_energy_time";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVEnergyTimeDerivative";
    kernel_name = prefix() + "pins_energy_time";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.template set<MooseFunctorName>(NS::density) = _density_name;
  params.template set<MooseFunctorName>(NS::cp) = _specific_heat_name;

  if (_porous_medium_treatment)
  {
    params.template set<MooseFunctorName>(NS::porosity) = _porosity_name;
    if (getProblem().hasFunctor(NS::time_deriv(_density_name), /*thread_id=*/0))
      params.template set<MooseFunctorName>(NS::time_deriv(NS::density)) =
          NS::time_deriv(_density_name);
    params.template set<bool>("is_solid") = false;
  }

  getProblem().addFVKernel(kernel_type, kernel_name, params);
}

template <class BaseType>
void
NSFVBase<BaseType>::addScalarTimeKernels()
{
  for (const auto & vname : _passive_scalar_names)
  {
    const std::string kernel_type = "FVFunctorTimeKernel";
    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.template set<NonlinearVariableName>("variable") = vname;

    getProblem().addFVKernel(kernel_type, prefix() + "ins_" + vname + "_time", params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSMassKernels()
{
  std::string kernel_type = "INSFVMassAdvection";
  std::string kernel_name = prefix() + "ins_mass_advection";
  std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMassAdvection";
    kernel_name = prefix() + "pins_mass_advection";
    rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.template set<NonlinearVariableName>("variable") = _pressure_name;
  params.template set<MooseFunctorName>(NS::density) = _density_name;
  params.template set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.template set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.template set<MooseEnum>("advected_interp_method") = _mass_advection_interpolation;

  getProblem().addFVKernel(kernel_type, kernel_name, params);

  if (parameters().template get<bool>("pin_pressure"))
  {
    MooseEnum pin_type = parameters().template get<MooseEnum>("pinned_pressure_type");
    std::string kernel_type;
    if (pin_type == "point-value")
      kernel_type = "FVPointValueConstraint";
    else
      kernel_type = "FVIntegralValueConstraint";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.template set<CoupledName>("lambda") = {"lambda"};
    params.template set<PostprocessorName>("phi0") =
        parameters().template get<PostprocessorName>("pinned_pressure_value");
    params.template set<NonlinearVariableName>("variable") = _pressure_name;
    if (pin_type == "point-value")
      params.template set<Point>("point") =
          parameters().template get<Point>("pinned_pressure_point");

    getProblem().addFVKernel(kernel_type, prefix() + "ins_mass_pressure_constraint", params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSMomentumAdvectionKernels()
{
  std::string kernel_type = "INSFVMomentumAdvection";
  std::string kernel_name = prefix() + "ins_momentum_advection_";
  std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumAdvection";
    kernel_name = prefix() + "pins_momentum_advection_";
    rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.template set<MooseFunctorName>(NS::density) = _density_name;
  params.template set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.template set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.template set<MooseEnum>("advected_interp_method") = _momentum_advection_interpolation;
  if (_porous_medium_treatment)
    params.template set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
    params.template set<MooseEnum>("momentum_component") = NS::directions[d];

    getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSMomentumViscousDissipationKernels()
{
  std::string kernel_type = "INSFVMomentumDiffusion";
  std::string kernel_name = prefix() + "ins_momentum_diffusion_";
  std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumDiffusion";
    kernel_name = prefix() + "pins_momentum_diffusion_";
    rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.template set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.template set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;

  if (_porous_medium_treatment)
    params.template set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
    params.template set<MooseEnum>("momentum_component") = NS::directions[d];

    getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSMomentumMixingLengthKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  const std::string kernel_type = "INSFVMixingLengthReynoldsStress";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.template set<MooseFunctorName>(NS::density) = _density_name;
  params.template set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;

  std::string kernel_name = prefix() + "ins_momentum_mixing_length_reynolds_stress_";
  std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_name = prefix() + "pins_momentum_mixing_length_reynolds_stress_";
    rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
  }

  params.template set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  for (unsigned int dim_i = 0; dim_i < _dim; ++dim_i)
    params.template set<MooseFunctorName>(u_names[dim_i]) = _velocity_name[dim_i];

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
    params.template set<MooseEnum>("momentum_component") = NS::directions[d];

    getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }

  const std::string ml_kernel_type = "WallDistanceMixingLengthAux";
  InputParameters ml_params = getFactory().getValidParams(ml_kernel_type);
  assignBlocks(ml_params, _blocks);
  ml_params.template set<AuxVariableName>("variable") = NS::mixing_length;
  ml_params.template set<std::vector<BoundaryName>>("walls") =
      parameters().template get<std::vector<BoundaryName>>("mixing_length_walls");
  if (parameters().isParamValid("mixing_length_aux_execute_on"))
    ml_params.template set<ExecFlagEnum>("execute_on") =
        parameters().template get<ExecFlagEnum>("mixing_length_aux_execute_on");
  else
    ml_params.template set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  ml_params.template set<Real>("von_karman_const") =
      parameters().template get<Real>("von_karman_const");
  ml_params.template set<Real>("von_karman_const_0") =
      parameters().template get<Real>("von_karman_const_0");
  ml_params.template set<Real>("delta") = parameters().template get<Real>("mixing_length_delta");

  getProblem().addAuxKernel(ml_kernel_type, prefix() + "mixing_length_aux ", ml_params);
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSMomentumPressureKernels()
{
  std::string kernel_type = "INSFVMomentumPressure";
  std::string kernel_name = prefix() + "ins_momentum_pressure_";
  std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVMomentumPressure";
    kernel_name = prefix() + "pins_momentum_pressure_";
    rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.template set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.template set<MooseFunctorName>("pressure") = _pressure_name;
  params.template set<bool>("correct_skewness") =
      parameters().template get<MooseEnum>("pressure_face_interpolation") == "skewness-corrected";
  if (_porous_medium_treatment)
    params.template set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.template set<MooseEnum>("momentum_component") = NS::directions[d];
    params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
    getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSMomentumGravityKernels()
{
  if (parameters().isParamValid("gravity"))
  {
    std::string kernel_type = "INSFVMomentumGravity";
    std::string kernel_name = prefix() + "ins_momentum_gravity_";
    std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

    if (_porous_medium_treatment)
    {
      kernel_type = "PINSFVMomentumGravity";
      kernel_name = prefix() + "pins_momentum_gravity_";
      rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
    }

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.template set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
    params.template set<MooseFunctorName>(NS::density) = _density_name;
    params.template set<RealVectorValue>("gravity") =
        parameters().template get<RealVectorValue>("gravity");
    if (_porous_medium_treatment)
      params.template set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      if (parameters().template get<RealVectorValue>("gravity")(d) != 0)
      {
        params.template set<MooseEnum>("momentum_component") = NS::directions[d];
        params.template set<NonlinearVariableName>("variable") = _velocity_name[d];

        getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
      }
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSMomentumBoussinesqKernels()
{
  if (parameters().isParamValid("gravity"))
  {
    std::string kernel_type = "INSFVMomentumBoussinesq";
    std::string kernel_name = prefix() + "ins_momentum_boussinesq_";
    std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";

    if (_porous_medium_treatment)
    {
      kernel_type = "PINSFVMomentumBoussinesq";
      kernel_name = prefix() + "pins_momentum_boussinesq_";
      rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
    }

    InputParameters params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.template set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
    params.template set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;
    params.template set<MooseFunctorName>(NS::density) = _density_name;
    params.template set<RealVectorValue>("gravity") =
        parameters().template get<RealVectorValue>("gravity");
    params.template set<Real>("ref_temperature") =
        parameters().template get<Real>("ref_temperature");
    params.template set<MooseFunctorName>("alpha_name") = _thermal_expansion_name;
    if (_porous_medium_treatment)
      params.template set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.template set<MooseEnum>("momentum_component") = NS::directions[d];
      params.template set<NonlinearVariableName>("variable") = _velocity_name[d];

      getProblem().addFVKernel(kernel_type, kernel_name + NS::directions[d], params);
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSMomentumFrictionKernels()
{
  unsigned int num_friction_blocks = _friction_blocks.size();
  unsigned int num_used_blocks = num_friction_blocks ? num_friction_blocks : 1;

  if (_porous_medium_treatment)
  {
    const std::string kernel_type = "PINSFVMomentumFriction";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.template set<MooseFunctorName>(NS::density) = _density_name;
    params.template set<UserObjectName>("rhie_chow_user_object") =
        prefix() + "pins_rhie_chow_interpolator";
    params.template set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

    for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
    {
      std::string block_name = "";
      if (num_friction_blocks)
      {
        params.template set<std::vector<SubdomainName>>("block") = _friction_blocks[block_i];
        block_name = Moose::stringify(_friction_blocks[block_i]);
      }
      else
      {
        assignBlocks(params, _blocks);
        block_name = std::to_string(block_i);
      }

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.template set<MooseEnum>("momentum_component") = NS::directions[d];
        for (unsigned int type_i = 0; type_i < _friction_types[block_i].size(); ++type_i)
        {
          const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
          if (upper_name == "DARCY")
            params.template set<MooseFunctorName>("Darcy_name") = _friction_coeffs[block_i][type_i];
          else if (upper_name == "FORCHHEIMER")
            params.template set<MooseFunctorName>("Forchheimer_name") =
                _friction_coeffs[block_i][type_i];
        }

        getProblem().addFVKernel(kernel_type,
                                 prefix() + "momentum_friction_" + block_name + "_" +
                                     NS::directions[d],
                                 params);
      }

      if (_use_friction_correction)
      {
        const std::string correction_kernel_type = "PINSFVMomentumFrictionCorrection";
        InputParameters corr_params = getFactory().getValidParams(correction_kernel_type);
        if (num_friction_blocks)
          corr_params.template set<std::vector<SubdomainName>>("block") = _friction_blocks[block_i];
        else
          assignBlocks(corr_params, _blocks);
        corr_params.template set<MooseFunctorName>(NS::density) = _density_name;
        corr_params.template set<UserObjectName>("rhie_chow_user_object") =
            prefix() + "pins_rhie_chow_interpolator";
        corr_params.template set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
        corr_params.template set<Real>("consistent_scaling") =
            parameters().template get<Real>("consistent_scaling");
        for (unsigned int d = 0; d < _dim; ++d)
        {
          corr_params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
          corr_params.template set<MooseEnum>("momentum_component") = NS::directions[d];
          for (unsigned int type_i = 0; type_i < _friction_types[block_i].size(); ++type_i)
          {
            const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
            if (upper_name == "DARCY")
              corr_params.template set<MooseFunctorName>("Darcy_name") =
                  _friction_coeffs[block_i][type_i];
            else if (upper_name == "FORCHHEIMER")
              corr_params.template set<MooseFunctorName>("Forchheimer_name") =
                  _friction_coeffs[block_i][type_i];
          }

          getProblem().addFVKernel(correction_kernel_type,
                                   prefix() + "pins_momentum_friction_correction_" + block_name +
                                       "_" + NS::directions[d],
                                   corr_params);
        }
      }
    }
  }
  else
  {
    const std::string kernel_type = "INSFVMomentumFriction";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.template set<UserObjectName>("rhie_chow_user_object") =
        prefix() + "ins_rhie_chow_interpolator";

    for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
    {
      std::string block_name = "";
      if (num_friction_blocks)
      {
        params.template set<std::vector<SubdomainName>>("block") = _friction_blocks[block_i];
        block_name = Moose::stringify(_friction_blocks[block_i]);
      }
      else
      {
        assignBlocks(params, _blocks);
        block_name = std::to_string(block_i);
      }

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.template set<MooseEnum>("momentum_component") = NS::directions[d];
        for (unsigned int type_i = 0; type_i < _friction_types[block_i].size(); ++type_i)
        {
          const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
          if (upper_name == "DARCY")
            params.template set<MooseFunctorName>("linear_coef_name") =
                _friction_coeffs[block_i][type_i];
          else if (upper_name == "FORCHHEIMER")
            params.template set<MooseFunctorName>("quadratic_coef_name") =
                _friction_coeffs[block_i][type_i];
        }

        getProblem().addFVKernel(kernel_type,
                                 prefix() + "ins_momentum_friction_" + block_name + "_" +
                                     NS::directions[d],
                                 params);
      }
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSEnergyAdvectionKernels()
{
  std::string kernel_type = "INSFVEnergyAdvection";
  std::string kernel_name = prefix() + "ins_energy_advection";
  std::string rhie_chow_name = prefix() + "ins_rhie_chow_interpolator";
  if (_porous_medium_treatment)
  {
    kernel_type = "PINSFVEnergyAdvection";
    kernel_name = prefix() + "pins_energy_advection";
    rhie_chow_name = prefix() + "pins_rhie_chow_interpolator";
  }

  InputParameters params = getFactory().getValidParams(kernel_type);
  params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  assignBlocks(params, _blocks);
  params.template set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
  params.template set<UserObjectName>("rhie_chow_user_object") = rhie_chow_name;
  params.template set<MooseEnum>("advected_interp_method") = _energy_advection_interpolation;

  getProblem().addFVKernel(kernel_type, kernel_name, params);
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSEnergyHeatConductionKernels()
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

      InputParameters params = getFactory().getValidParams(kernel_type);
      params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      std::vector<SubdomainName> block_names =
          num_blocks ? _thermal_conductivity_blocks[block_i] : _blocks;
      assignBlocks(params, block_names);
      std::string conductivity_name = vector_conductivity ? NS::kappa : NS::k;
      params.template set<MooseFunctorName>(conductivity_name) =
          _thermal_conductivity_name[block_i];
      params.template set<MooseFunctorName>(NS::porosity) = _porosity_name;

      getProblem().addFVKernel(
          kernel_type, prefix() + "pins_energy_diffusion_" + block_name, params);
    }
    else
    {
      const std::string kernel_type = "FVDiffusion";
      InputParameters params = getFactory().getValidParams(kernel_type);
      params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      std::vector<SubdomainName> block_names =
          num_blocks ? _thermal_conductivity_blocks[block_i] : _blocks;
      assignBlocks(params, block_names);
      params.template set<MooseFunctorName>("coeff") = _thermal_conductivity_name[block_i];

      getProblem().addFVKernel(
          kernel_type, prefix() + "ins_energy_diffusion_" + block_name, params);
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSEnergyAmbientConvection()
{
  unsigned int num_convection_blocks = _ambient_convection_blocks.size();
  unsigned int num_used_blocks = num_convection_blocks ? num_convection_blocks : 1;

  const std::string kernel_type = "PINSFVEnergyAmbientConvection";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.template set<MooseFunctorName>(NS::T_fluid) = _fluid_temperature_name;
  params.template set<bool>("is_solid") = false;

  for (unsigned int block_i = 0; block_i < num_used_blocks; ++block_i)
  {
    std::string block_name = "";
    if (num_convection_blocks)
    {
      params.template set<std::vector<SubdomainName>>("block") =
          _ambient_convection_blocks[block_i];
      block_name = Moose::stringify(_ambient_convection_blocks[block_i]);
    }
    else
    {
      assignBlocks(params, _blocks);
      block_name = std::to_string(block_i);
    }

    params.template set<MooseFunctorName>("h_solid_fluid") = _ambient_convection_alpha[block_i];
    params.template set<MooseFunctorName>(NS::T_solid) = _ambient_temperature[block_i];

    getProblem().addFVKernel(kernel_type, prefix() + "ambient_convection_" + block_name, params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSEnergyExternalHeatSource()
{
  const std::string kernel_type = "FVCoupledForce";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  assignBlocks(params, _blocks);
  params.template set<MooseFunctorName>("v") =
      parameters().template get<MooseFunctorName>("external_heat_source");
  params.template set<Real>("coef") = parameters().template get<Real>("external_heat_source_coeff");

  getProblem().addFVKernel(kernel_type, prefix() + "external_heat_source", params);
}

template <class BaseType>
void
NSFVBase<BaseType>::addScalarAdvectionKernels()
{
  for (const auto & vname : _passive_scalar_names)
  {
    const std::string kernel_type = "INSFVScalarFieldAdvection";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.template set<NonlinearVariableName>("variable") = vname;
    params.template set<MooseEnum>("velocity_interp_method") = _velocity_interpolation;
    params.template set<MooseEnum>("advected_interp_method") =
        _passive_scalar_advection_interpolation;

    if (_porous_medium_treatment)
      params.template set<UserObjectName>("rhie_chow_user_object") =
          prefix() + "pins_rhie_chow_interpolator";
    else
      params.template set<UserObjectName>("rhie_chow_user_object") =
          prefix() + "ins_rhie_chow_interpolator";

    getProblem().addFVKernel(kernel_type, prefix() + "ins_" + vname + "_advection", params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addScalarMixingLengthKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  const std::string kernel_type = "INSFVMixingLengthScalarDiffusion";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.template set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;

  for (unsigned int dim_i = 0; dim_i < _dim; ++dim_i)
    params.template set<MooseFunctorName>(u_names[dim_i]) = _velocity_name[dim_i];

  for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
  {
    params.template set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
    if (_passive_scalar_schmidt_number.size())
      params.template set<Real>("schmidt_number") = _passive_scalar_schmidt_number[name_i];
    else
      params.template set<Real>("schmidt_number") = 1.0;

    getProblem().addFVKernel(
        kernel_type, prefix() + _passive_scalar_names[name_i] + "_mixing_length", params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addScalarDiffusionKernels()
{
  for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
  {
    const std::string kernel_type = "FVDiffusion";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.template set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
    assignBlocks(params, _blocks);
    params.template set<MooseFunctorName>("coeff") = _passive_scalar_diffusivity[name_i];

    getProblem().addFVKernel(
        kernel_type, prefix() + "ins_" + _passive_scalar_names[name_i] + "_diffusion", params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addScalarSourceKernels()
{
  for (unsigned int name_i = 0; name_i < _passive_scalar_names.size(); ++name_i)
  {
    const std::string kernel_type = "FVBodyForce";
    InputParameters params = getFactory().getValidParams(kernel_type);
    params.template set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
    assignBlocks(params, _blocks);
    params.template set<FunctionName>("function") = _passive_scalar_source[name_i];

    getProblem().addFVKernel(
        kernel_type, prefix() + "ins_" + _passive_scalar_names[name_i] + "_source", params);
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addScalarCoupledSourceKernels()
{
  for (unsigned int name_eq = 0; name_eq < _passive_scalar_names.size(); name_eq++)
  {
    for (unsigned int i = 0; i < _passive_scalar_coupled_source[name_eq].size(); ++i)
    {
      const std::string kernel_type = "FVCoupledForce";
      InputParameters params = getFactory().getValidParams(kernel_type);
      params.template set<NonlinearVariableName>("variable") = _passive_scalar_names[name_eq];
      assignBlocks(params, _blocks);
      params.template set<MooseFunctorName>("v") = _passive_scalar_coupled_source[name_eq][i];
      params.template set<Real>("coef") = _passive_scalar_coupled_source_coeff[name_eq][i];

      getProblem().addFVKernel(kernel_type,
                               prefix() + "ins_" + _passive_scalar_names[name_eq] +
                                   "_coupled_source_" + std::to_string(i),
                               params);
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSInletBC()
{
  unsigned int flux_bc_counter = 0;
  unsigned int velocity_pressure_counter = 0;
  for (unsigned int bc_ind = 0; bc_ind < _inlet_boundaries.size(); ++bc_ind)
  {
    if (_momentum_inlet_types[bc_ind] == "fixed-velocity")
    {
      const std::string bc_type = "INSFVInletVelocityBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.template set<FunctionName>("function") =
            _momentum_inlet_function[velocity_pressure_counter][d];

        getProblem().addFVBC(bc_type, _velocity_name[d] + "_" + _inlet_boundaries[bc_ind], params);
      }
      ++velocity_pressure_counter;
    }
    else if (_momentum_inlet_types[bc_ind] == "fixed-pressure")
    {
      const std::string bc_type = "INSFVOutletPressureBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<NonlinearVariableName>("variable") = _pressure_name;
      params.template set<FunctionName>("function") =
          _momentum_inlet_function[velocity_pressure_counter][0];
      params.template set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      getProblem().addFVBC(bc_type, _pressure_name + "_" + _inlet_boundaries[bc_ind], params);
      ++velocity_pressure_counter;
    }
    else if (_momentum_inlet_types[bc_ind] == "flux-mass" ||
             _momentum_inlet_types[bc_ind] == "flux-velocity")
    {
      {
        const std::string bc_type =
            _porous_medium_treatment ? "PWCNSFVMomentumFluxBC" : "WCNSFVMomentumFluxBC";
        InputParameters params = getFactory().getValidParams(bc_type);

        if (_flux_inlet_directions.size())
          params.template set<Point>("direction") = _flux_inlet_directions[flux_bc_counter];

        params.template set<MooseFunctorName>(NS::density) = _density_name;
        params.template set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};
        if (_porous_medium_treatment)
        {
          params.template set<UserObjectName>("rhie_chow_user_object") =
              prefix() + "pins_rhie_chow_interpolator";
          params.template set<MooseFunctorName>(NS::porosity) = _porosity_name;
        }
        else
          params.template set<UserObjectName>("rhie_chow_user_object") =
              prefix() + "ins_rhie_chow_interpolator";

        if (_momentum_inlet_types[bc_ind] == "flux-mass")
        {
          params.template set<PostprocessorName>("mdot_pp") = _flux_inlet_pps[flux_bc_counter];
          params.template set<PostprocessorName>("area_pp") =
              "area_pp_" + _inlet_boundaries[bc_ind];
        }
        else
          params.template set<PostprocessorName>("velocity_pp") = _flux_inlet_pps[flux_bc_counter];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          params.template set<MooseEnum>("momentum_component") = NS::directions[d];
          params.template set<NonlinearVariableName>("variable") = _velocity_name[d];

          getProblem().addFVBC(
              bc_type, _velocity_name[d] + "_" + _inlet_boundaries[bc_ind], params);
        }
      }
      {
        const std::string bc_type = "WCNSFVMassFluxBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.template set<MooseFunctorName>(NS::density) = _density_name;
        params.template set<NonlinearVariableName>("variable") = _pressure_name;
        params.template set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

        if (_flux_inlet_directions.size())
          params.template set<Point>("direction") = _flux_inlet_directions[flux_bc_counter];

        if (_momentum_inlet_types[bc_ind] == "flux-mass")
        {
          params.template set<PostprocessorName>("mdot_pp") = _flux_inlet_pps[flux_bc_counter];
          params.template set<PostprocessorName>("area_pp") =
              "area_pp_" + _inlet_boundaries[bc_ind];
        }
        else
          params.template set<PostprocessorName>("velocity_pp") = _flux_inlet_pps[flux_bc_counter];

        getProblem().addFVBC(bc_type, _pressure_name + "_" + _inlet_boundaries[bc_ind], params);
      }

      // need to increment flux_bc_counter
      ++flux_bc_counter;
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSEnergyInletBC()
{
  unsigned int flux_bc_counter = 0;
  for (unsigned int bc_ind = 0; bc_ind < _inlet_boundaries.size(); ++bc_ind)
  {
    if (_energy_inlet_types[bc_ind] == "fixed-temperature")
    {
      const std::string bc_type = "FVFunctionDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.template set<FunctionName>("function") = _energy_inlet_function[bc_ind];
      params.template set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + _inlet_boundaries[bc_ind], params);
    }
    else if (_energy_inlet_types[bc_ind] == "heatflux")
    {
      const std::string bc_type = "FVFunctionNeumannBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.template set<FunctionName>("function") = _energy_inlet_function[bc_ind];
      params.template set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + _inlet_boundaries[bc_ind], params);
    }
    else if (_energy_inlet_types[bc_ind] == "flux-mass" ||
             _energy_inlet_types[bc_ind] == "flux-velocity")
    {
      const std::string bc_type = "WCNSFVEnergyFluxBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      if (_flux_inlet_directions.size())
        params.template set<Point>("direction") = _flux_inlet_directions[flux_bc_counter];
      if (_energy_inlet_types[bc_ind] == "flux-mass")
      {
        params.template set<PostprocessorName>("mdot_pp") = _flux_inlet_pps[flux_bc_counter];
        params.template set<PostprocessorName>("area_pp") = "area_pp_" + _inlet_boundaries[bc_ind];
      }
      else
        params.template set<PostprocessorName>("velocity_pp") = _flux_inlet_pps[flux_bc_counter];

      params.template set<PostprocessorName>("temperature_pp") = _energy_inlet_function[bc_ind];
      params.template set<MooseFunctorName>(NS::density) = _density_name;
      params.template set<MooseFunctorName>(NS::cp) = _specific_heat_name;

      params.template set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + _inlet_boundaries[bc_ind], params);
      flux_bc_counter += 1;
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addScalarInletBC()
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
        InputParameters params = getFactory().getValidParams(bc_type);
        params.template set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
        params.template set<FunctionName>("function") =
            _passive_scalar_inlet_function[name_i][bc_ind];
        params.template set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

        getProblem().addFVBC(
            bc_type, _passive_scalar_names[name_i] + "_" + _inlet_boundaries[bc_ind], params);
      }
      else if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-mass" ||
               _passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-velocity")
      {
        const std::string bc_type = "WCNSFVScalarFluxBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.template set<NonlinearVariableName>("variable") = _passive_scalar_names[name_i];
        if (_flux_inlet_directions.size())
          params.template set<Point>("direction") = _flux_inlet_directions[flux_bc_counter];
        if (_passive_scalar_inlet_types[name_i * num_inlets + bc_ind] == "flux-mass")
        {
          params.template set<PostprocessorName>("mdot_pp") = _flux_inlet_pps[flux_bc_counter];
          params.template set<PostprocessorName>("area_pp") =
              "area_pp_" + _inlet_boundaries[bc_ind];
          params.template set<MooseFunctorName>(NS::density) = _density_name;
        }
        else
          params.template set<PostprocessorName>("velocity_pp") = _flux_inlet_pps[flux_bc_counter];

        params.template set<PostprocessorName>("scalar_value_pp") =
            _passive_scalar_inlet_function[name_i][bc_ind];
        params.template set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

        getProblem().addFVBC(
            bc_type, _passive_scalar_names[name_i] + "_" + _inlet_boundaries[bc_ind], params);
        flux_bc_counter += 1;
      }
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSOutletBC()
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
        InputParameters params = getFactory().getValidParams(bc_type);
        params.template set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};
        params.template set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;
        params.template set<UserObjectName>("rhie_chow_user_object") =
            prefix() + "pins_rhie_chow_interpolator";
        params.template set<MooseFunctorName>(NS::density) = _density_name;

        for (unsigned int i = 0; i < _dim; ++i)
          params.template set<MooseFunctorName>(u_names[i]) = _velocity_name[i];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
          params.template set<MooseEnum>("momentum_component") = NS::directions[d];

          getProblem().addFVBC(
              bc_type, _velocity_name[d] + "_" + _outlet_boundaries[bc_ind], params);
        }
      }
      else
      {
        const std::string bc_type = "INSFVMomentumAdvectionOutflowBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.template set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};
        params.template set<UserObjectName>("rhie_chow_user_object") =
            prefix() + "ins_rhie_chow_interpolator";
        params.template set<MooseFunctorName>(NS::density) = _density_name;

        for (unsigned int i = 0; i < _dim; ++i)
          params.template set<MooseFunctorName>(u_names[i]) = _velocity_name[i];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
          params.template set<MooseEnum>("momentum_component") = NS::directions[d];

          getProblem().addFVBC(
              bc_type, _velocity_name[d] + "_" + _outlet_boundaries[bc_ind], params);
        }
      }
    }

    if (_momentum_outlet_types[bc_ind] == "fixed-pressure" ||
        _momentum_outlet_types[bc_ind] == "fixed-pressure-zero-gradient")
    {
      const std::string bc_type = "INSFVOutletPressureBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<NonlinearVariableName>("variable") = _pressure_name;
      params.template set<FunctionName>("function") = _pressure_function[bc_ind];
      params.template set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

      getProblem().addFVBC(bc_type, _pressure_name + "_" + _outlet_boundaries[bc_ind], params);
    }
    else if (_momentum_outlet_types[bc_ind] == "zero-gradient")
    {
      const std::string bc_type = "INSFVMassAdvectionOutflowBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<NonlinearVariableName>("variable") = _pressure_name;
      params.template set<MooseFunctorName>(NS::density) = _density_name;
      params.template set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
        params.template set<MooseFunctorName>(u_names[d]) = _velocity_name[d];

      getProblem().addFVBC(bc_type, _pressure_name + "_" + _outlet_boundaries[bc_ind], params);
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSWallBC()
{
  const std::string u_names[3] = {"u", "v", "w"};

  for (unsigned int bc_ind = 0; bc_ind < _wall_boundaries.size(); ++bc_ind)
  {
    if (_momentum_wall_types[bc_ind] == "noslip")
    {
      const std::string bc_type = "INSFVNoSlipWallBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.template set<FunctionName>("function") = "0";

        getProblem().addFVBC(bc_type, _velocity_name[d] + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (_momentum_wall_types[bc_ind] == "wallfunction")
    {
      const std::string bc_type = "INSFVWallFunctionBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
      params.template set<MooseFunctorName>(NS::density) = _density_name;
      params.template set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      if (_porous_medium_treatment)
        params.template set<UserObjectName>("rhie_chow_user_object") =
            prefix() + "pins_rhie_chow_interpolator";
      else
        params.template set<UserObjectName>("rhie_chow_user_object") =
            prefix() + "ins_rhie_chow_interpolator";

      for (unsigned int d = 0; d < _dim; ++d)
        params.template set<MooseFunctorName>(u_names[d]) = _velocity_name[d];

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.template set<MooseEnum>("momentum_component") = NS::directions[d];

        getProblem().addFVBC(bc_type, _velocity_name[d] + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (_momentum_wall_types[bc_ind] == "slip")
    {
      const std::string bc_type = "INSFVNaturalFreeSlipBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        if (_porous_medium_treatment)
          params.template set<UserObjectName>("rhie_chow_user_object") =
              prefix() + "pins_rhie_chow_interpolator";
        else
          params.template set<UserObjectName>("rhie_chow_user_object") =
              prefix() + "ins_rhie_chow_interpolator";

        params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
        params.template set<MooseEnum>("momentum_component") = NS::directions[d];

        getProblem().addFVBC(bc_type, _velocity_name[d] + "_" + _wall_boundaries[bc_ind], params);
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

        InputParameters params = getFactory().getValidParams(bc_type);
        params.template set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

        MooseFunctorName viscosity_name = _dynamic_viscosity_name;
        if (_turbulence_handling != "none")
          viscosity_name = NS::total_viscosity;
        params.template set<MooseFunctorName>(NS::mu) = viscosity_name;

        for (unsigned int d = 0; d < _dim; ++d)
          params.template set<MooseFunctorName>(u_names[d]) = _velocity_name[d];

        for (unsigned int d = 0; d < _dim; ++d)
        {
          if (_porous_medium_treatment)
            params.template set<UserObjectName>("rhie_chow_user_object") =
                prefix() + "pins_rhie_chow_interpolator";
          else
            params.template set<UserObjectName>("rhie_chow_user_object") =
                prefix() + "ins_rhie_chow_interpolator";

          params.template set<NonlinearVariableName>("variable") = _velocity_name[d];
          params.template set<MooseEnum>("momentum_component") = NS::directions[d];

          getProblem().addFVBC(bc_type, _velocity_name[d] + "_" + _wall_boundaries[bc_ind], params);
        }
      }
      {
        const std::string bc_type = "INSFVSymmetryPressureBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.template set<NonlinearVariableName>("variable") = _pressure_name;
        params.template set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

        getProblem().addFVBC(bc_type, _pressure_name + "_" + _wall_boundaries[bc_ind], params);
      }
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addINSEnergyWallBC()
{
  for (unsigned int bc_ind = 0; bc_ind < _wall_boundaries.size(); ++bc_ind)
  {
    if (_energy_wall_types[bc_ind] == "fixed-temperature")
    {
      const std::string bc_type = "FVFunctionDirichletBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.template set<FunctionName>("function") = _energy_wall_function[bc_ind];
      params.template set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + _wall_boundaries[bc_ind], params);
    }
    else if (_energy_wall_types[bc_ind] == "heatflux")
    {
      const std::string bc_type = "FVFunctionNeumannBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
      params.template set<FunctionName>("function") = _energy_wall_function[bc_ind];
      params.template set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      getProblem().addFVBC(
          bc_type, _fluid_temperature_name + "_" + _wall_boundaries[bc_ind], params);
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addWCNSMassTimeKernels()
{
  std::string mass_kernel_type = "WCNSFVMassTimeDerivative";
  std::string kernel_name = prefix() + "wcns_mass_time";

  if (_porous_medium_treatment)
  {
    mass_kernel_type = "PWCNSFVMassTimeDerivative";
    kernel_name = prefix() + "pwcns_mass_time";
  }

  InputParameters params = getFactory().getValidParams(mass_kernel_type);
  assignBlocks(params, _blocks);
  params.template set<NonlinearVariableName>("variable") = _pressure_name;
  params.template set<MooseFunctorName>(NS::time_deriv(NS::density)) =
      NS::time_deriv(_density_name);
  if (_porous_medium_treatment)
    params.template set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  getProblem().addFVKernel(mass_kernel_type, kernel_name, params);
}

template <class BaseType>
void
NSFVBase<BaseType>::addWCNSMomentumTimeKernels()
{
  const std::string mom_kernel_type = "WCNSFVMomentumTimeDerivative";
  InputParameters params = getFactory().getValidParams(mom_kernel_type);
  assignBlocks(params, _blocks);
  params.template set<MooseFunctorName>(NS::density) = _density_name;
  params.template set<MooseFunctorName>(NS::time_deriv(NS::density)) =
      NS::time_deriv(_density_name);

  for (unsigned int d = 0; d < _dim; ++d)
  {
    params.template set<MooseEnum>("momentum_component") = NS::directions[d];
    params.template set<NonlinearVariableName>("variable") = _velocity_name[d];

    if (_porous_medium_treatment)
    {
      params.template set<UserObjectName>("rhie_chow_user_object") =
          prefix() + "pins_rhie_chow_interpolator";
      getProblem().addFVKernel(
          mom_kernel_type, prefix() + "pwcns_momentum_" + NS::directions[d] + "_time", params);
    }
    else
    {
      params.template set<UserObjectName>("rhie_chow_user_object") =
          prefix() + "ins_rhie_chow_interpolator";
      getProblem().addFVKernel(
          mom_kernel_type, prefix() + "wcns_momentum_" + NS::directions[d] + "_time", params);
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::addWCNSEnergyTimeKernels()
{
  std::string en_kernel_type = "WCNSFVEnergyTimeDerivative";
  std::string kernel_name = prefix() + "wcns_energy_time";

  if (_porous_medium_treatment)
  {
    en_kernel_type = "PINSFVEnergyTimeDerivative";
    kernel_name = prefix() + "pwcns_energy_time";
  }

  InputParameters params = getFactory().getValidParams(en_kernel_type);
  assignBlocks(params, _blocks);
  params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;
  params.template set<MooseFunctorName>(NS::density) = _density_name;
  params.template set<MooseFunctorName>(NS::time_deriv(NS::density)) =
      NS::time_deriv(_density_name);
  params.template set<MooseFunctorName>(NS::cp) = _specific_heat_name;

  if (_porous_medium_treatment)
  {
    params.template set<MooseFunctorName>(NS::porosity) = _porosity_name;
    params.template set<bool>("is_solid") = false;
  }

  getProblem().addFVKernel(en_kernel_type, kernel_name, params);
}

template <class BaseType>
void
NSFVBase<BaseType>::addWCNSEnergyMixingLengthKernels()
{
  const std::string u_names[3] = {"u", "v", "w"};
  const std::string kernel_type = "WCNSFVMixingLengthEnergyDiffusion";
  InputParameters params = getFactory().getValidParams(kernel_type);
  assignBlocks(params, _blocks);
  params.template set<MooseFunctorName>(NS::density) = _density_name;
  params.template set<MooseFunctorName>(NS::cp) = _specific_heat_name;
  params.template set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;
  params.template set<Real>("schmidt_number") =
      parameters().template get<Real>("turbulent_prandtl");
  params.template set<NonlinearVariableName>("variable") = _fluid_temperature_name;

  for (unsigned int dim_i = 0; dim_i < _dim; ++dim_i)
    params.template set<MooseFunctorName>(u_names[dim_i]) = _velocity_name[dim_i];

  if (_porous_medium_treatment)
    getProblem().addFVKernel(kernel_type, prefix() + "pins_energy_mixing_length_diffusion", params);
  else
    getProblem().addFVKernel(kernel_type, prefix() + "ins_energy_mixing_length_diffusion", params);
}

template <class BaseType>
void
NSFVBase<BaseType>::addEnthalpyMaterial()
{
  InputParameters params = getFactory().getValidParams("INSFVEnthalpyMaterial");
  assignBlocks(params, _blocks);

  params.template set<MooseFunctorName>(NS::density) = _density_name;
  params.template set<MooseFunctorName>(NS::cp) = _specific_heat_name;
  params.template set<MooseFunctorName>("temperature") = _fluid_temperature_name;

  getProblem().addMaterial("INSFVEnthalpyMaterial", prefix() + "ins_enthalpy_material", params);
}

template <class BaseType>
void
NSFVBase<BaseType>::addPorousMediumSpeedMaterial()
{
  InputParameters params = getFactory().getValidParams("PINSFVSpeedFunctorMaterial");
  assignBlocks(params, _blocks);

  for (unsigned int dim_i = 0; dim_i < _dim; ++dim_i)
    params.template set<MooseFunctorName>(NS::superficial_velocity_vector[dim_i]) =
        _velocity_name[dim_i];
  params.template set<MooseFunctorName>(NS::porosity) = _flow_porosity_functor_name;

  getProblem().addMaterial("PINSFVSpeedFunctorMaterial", prefix() + "pins_speed_material", params);
}

template <class BaseType>
void
NSFVBase<BaseType>::addMixingLengthMaterial()
{
  const std::string u_names[3] = {"u", "v", "w"};
  InputParameters params = getFactory().getValidParams("MixingLengthTurbulentViscosityMaterial");
  assignBlocks(params, _blocks);

  for (unsigned int d = 0; d < _dim; ++d)
    params.template set<MooseFunctorName>(u_names[d]) = _velocity_name[d];

  params.template set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;

  params.template set<MooseFunctorName>(NS::density) = _density_name;
  params.template set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;

  getProblem().addMaterial(
      "MixingLengthTurbulentViscosityMaterial", prefix() + "mixing_length_material", params);
}

template <class BaseType>
void
NSFVBase<BaseType>::addBoundaryPostprocessors()
{
  for (unsigned int bc_ind = 0; bc_ind < _inlet_boundaries.size(); ++bc_ind)
    if (_momentum_inlet_types[bc_ind] == "flux-mass" ||
        (_has_energy_equation && _momentum_inlet_types[bc_ind] == "flux-velocity"))
    {
      const std::string pp_type = "AreaPostprocessor";
      InputParameters params = getFactory().getValidParams(pp_type);
      params.template set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};
      params.template set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

      getProblem().addPostprocessor(pp_type, "area_pp_" + _inlet_boundaries[bc_ind], params);
    }
}

template <class BaseType>
InputParameters
NSFVBase<BaseType>::getGhostParametersForRM()
{
  unsigned short necessary_layers = parameters().template get<unsigned short>("ghost_layers");
  if (_momentum_face_interpolation == "skewness-corrected" ||
      _energy_face_interpolation == "skewness-corrected" ||
      _pressure_face_interpolation == "skewness-corrected" ||
      _turbulence_handling == "mixing-length" ||
      (_porous_medium_treatment && parameters().template get<MooseEnum>(
                                       "porosity_interface_pressure_treatment") != "automatic"))
    necessary_layers = std::max(necessary_layers, (unsigned short)3);

  if (_porous_medium_treatment && parameters().isParamValid("porosity_smoothing_layers"))
    necessary_layers = std::max(
        parameters().template get<unsigned short>("porosity_smoothing_layers"), necessary_layers);

  const std::string kernel_type = "INSFVMixingLengthReynoldsStress";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.template set<unsigned short>("ghost_layers") = necessary_layers;

  return params;
}

template <class BaseType>
void
NSFVBase<BaseType>::processMesh()
{
  getProblem().needFV();

  _blocks = getBlocks();

  // If the user doesn't define a block name we go with the default
  if (!_blocks.size())
  {
    _blocks.push_back("ANY_BLOCK_ID");
    _dim = getMesh().dimension();
  }
  else
    _dim = getMesh().getBlocksMaxDimension(_blocks);
}

template <class BaseType>
void
NSFVBase<BaseType>::assignBlocks(InputParameters & params,
                                 const std::vector<SubdomainName> & blocks)
{
  // We only set the blocks if we don't have `ANY_BLOCK_ID` defined because the subproblem
  // (throug the mesh) errors out if we use this keyword during the addVariable/Kernel
  // functions
  if (std::find(blocks.begin(), blocks.end(), "ANY_BLOCK_ID") == blocks.end())
    params.template set<std::vector<SubdomainName>>("block") = blocks;
}

template <class BaseType>
void
NSFVBase<BaseType>::processVariables()
{
  _create_scalar_variable.clear();
  for (const auto & it : _passive_scalar_names)
    if (getProblem().hasVariable(it))
      _create_scalar_variable.push_back(false);
    else
      _create_scalar_variable.push_back(true);

  if ((_velocity_name.size() != _dim) && (_velocity_name.size() != 3))
    mooseError("The number of velocity variable names supplied to the NSFVAction is not " +
               std::to_string(_dim) + " (mesh dimension) or 3!");

  if (!_create_velocity)
    for (const auto & vname : _velocity_name)
    {
      if (!(getProblem().hasVariable(vname)))
        paramError("velocity_variable",
                   "Variable (" + vname +
                       ") supplied to the NavierStokesFV action does not exist!");
      else
        checkVariableBlockConsistency(vname);
    }

  if (!_create_pressure)
  {
    if (!(getProblem().hasVariable(_pressure_name)))
      paramError("pressure_variable",
                 "Variable (" + _pressure_name +
                     ") supplied to the NavierStokesFV action does not exist!");
    else
      checkVariableBlockConsistency(_pressure_name);
  }

  if (!_create_fluid_temperature)
    if (!(getProblem().hasVariable(_fluid_temperature_name)))
      paramError("pressure_variable",
                 "Variable (" + _fluid_temperature_name +
                     ") supplied to the NavierStokesFV action does not exist!");
}

template <class BaseType>
void
NSFVBase<BaseType>::checkVariableBlockConsistency(const std::string & var_name)
{
  const auto & fv_variable =
      dynamic_cast<const MooseVariableFVReal &>(getProblem().getVariable(0, var_name));
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

template <class BaseType>
bool
NSFVBase<BaseType>::processThermalConductivity()
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
      if (getProblem().template hasFunctorWithType<ADReal>(_thermal_conductivity_name[i],
                                                           /*thread_id=*/0))
        have_scalar = true;
      else
      {
        if (getProblem().template hasFunctorWithType<ADRealVectorValue>(
                _thermal_conductivity_name[i],
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

template <class BaseType>
void
NSFVBase<BaseType>::checkGeneralControlErrors()
{
  if (_compressibility == "weakly-compressible" && _boussinesq_approximation == true)
    paramError("boussinesq_approximation",
               "We cannot use boussinesq approximation while running in weakly-compressible mode!");

  if (parameters().isParamValid("porosity_smoothing_layers"))
  {
    if (!_porous_medium_treatment)
      paramError(
          "porosity_smoothing_layers",
          "This parameter should not be defined if the porous medium treatment is disabled!");

    if (parameters().template get<unsigned short>("porosity_smoothing_layers") != 0 &&
        parameters().template get<MooseEnum>("porosity_interface_pressure_treatment") !=
            "automatic")
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

  if (parameters().isParamValid("consistent_scaling") && !_use_friction_correction)
    paramError("consistent_scaling",
               "Consistent scaling should not be defined if friction correction is disabled!");

  if (parameters().template get<bool>("pin_pressure"))
  {
    checkDependentParameterError("pin_pressure", {"pinned_pressure_type"}, true);

    MooseEnum pin_type = parameters().template get<MooseEnum>("pinned_pressure_type");
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

  if (parameters().template get<MooseEnum>("porosity_interface_pressure_treatment") != "bernoulli")
    checkDependentParameterError("porosity_interface_pressure_treatment",
                                 {"pressure_allow_expansion_on_bernoulli_faces"});
}

template <class BaseType>
void
NSFVBase<BaseType>::checkICParameterErrors()
{
  if (parameters().isParamValid("initial_scalar_variables"))
  {
    unsigned int num_created_variables = 0;
    for (const auto & it : _create_scalar_variable)
      if (it == true)
        num_created_variables += 1;
    auto num_init_conditions =
        parameters().template get<std::vector<FunctionName>>("initial_scalar_variables").size();
    if (num_created_variables != num_init_conditions)
      paramError("initial_scalar_variables",
                 "The number of initial conditions (" + std::to_string(num_init_conditions) +
                     ") is not equal to the number of self-generated variables (" +
                     std::to_string(num_created_variables) + ") !");
  }

  auto vvalue = parameters().template get<std::vector<FunctionName>>("initial_velocity");
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

  if (parameters().template get<bool>("initialize_variables_from_mesh_file"))
    checkDependentParameterError("initialize_variables_from_mesh_file",
                                 {"initial_velocity",
                                  "initial_pressure",
                                  "initial_temperature",
                                  "initial_scalar_variables"});
}

template <class BaseType>
void
NSFVBase<BaseType>::checkBoundaryParameterErrors()
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
    if (num_pressure_outlets == 0 && !(parameters().template get<bool>("pin_pressure")))
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

template <class BaseType>
void
NSFVBase<BaseType>::checkAmbientConvectionParameterErrors()
{
  if (_has_energy_equation)
  {
    checkBlockwiseConsistency<MooseFunctorName>(
        "ambient_convection_blocks", {"ambient_convection_alpha", "ambient_temperature"});
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::checkFrictionParameterErrors()
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

template <class BaseType>
void
NSFVBase<BaseType>::checkPassiveScalarParameterErrors()
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

template <class BaseType>
template <typename T>
void
NSFVBase<BaseType>::checkBlockwiseConsistency(const std::string block_param_name,
                                              const std::vector<std::string> parameter_names)
{
  const std::vector<std::vector<SubdomainName>> & block_names =
      parameters().template get<std::vector<std::vector<SubdomainName>>>(block_param_name);

  if (block_names.size())
  {
    // We only check block-restrictions if the action is not restricted to `ANY_BLOCK_ID`.
    // If the users define blocks that are not on the mesh, they will receive errors from the
    // objects created created by the action
    if (std::find(_blocks.begin(), _blocks.end(), "ANY_BLOCK_ID") == _blocks.end())
      for (const auto & block_group : block_names)
        for (const auto & block : block_group)
          if (std::find(_blocks.begin(), _blocks.end(), block) == _blocks.end())
            paramError(block_param_name,
                       "Block '" + block +
                           "' is not present in the block restriction of the fluid flow action!");

    for (const auto & param_name : parameter_names)
    {
      const std::vector<T> & param_vector = parameters().template get<std::vector<T>>(param_name);
      if (block_names.size() != param_vector.size())
        paramError(param_name,
                   "The number of entries in '" + param_name + "' (" +
                       std::to_string(param_vector.size()) +
                       ") is not the same as the number of blocks"
                       " (" +
                       std::to_string(block_names.size()) + ") in '" + block_param_name + "'!");
    }
  }
  else
  {
    unsigned int previous_size = 0;
    for (unsigned int param_i = 0; param_i < parameter_names.size(); ++param_i)
    {
      const std::vector<T> & param_vector =
          parameters().template get<std::vector<T>>(parameter_names[param_i]);
      if (param_i == 0)
      {
        if (param_vector.size() > 1)
          paramError(parameter_names[param_i],
                     "The user should only use one or zero entries in " + parameter_names[param_i] +
                         " if " + block_param_name + " not defined!");
        previous_size = param_vector.size();
      }
      else
      {
        if (previous_size != param_vector.size())
          paramError(parameter_names[param_i],
                     "The number of entries in '" + parameter_names[param_i] +
                         "' is not the same as the number of entries in '" +
                         parameter_names[param_i - 1] + "'!");
      }
    }
  }
}

template <class BaseType>
void
NSFVBase<BaseType>::checkDependentParameterError(
    const std::string & main_parameter,
    const std::vector<std::string> & dependent_parameters,
    const bool should_be_defined)
{
  for (const auto & param : dependent_parameters)
    if (parameters().isParamSetByUser(param) == !should_be_defined)
      paramError(param,
                 "This parameter should " + std::string(should_be_defined ? "" : "not") +
                     " be given by the user with the corresponding " + main_parameter +
                     " setting!");
}

template <class BaseType>
void
NSFVBase<BaseType>::checkSizeFriendParams(const unsigned int param_vector_1_size,
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

template <class BaseType>
void
NSFVBase<BaseType>::checkSizeParam(const unsigned int param_vector_size,
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

template <class BaseType>
void
NSFVBase<BaseType>::checkRhieChowFunctorsDefined()
{
  if (!getProblem().hasFunctor("ax", /*thread_id=*/0))
    paramError("add_flow_equations",
               "Rhie Chow coefficient ax must be provided for advection by auxiliary velocities");
  if (_dim >= 2 && !getProblem().hasFunctor("ay", /*thread_id=*/0))
    paramError("add_flow_equations",
               "Rhie Chow coefficient ay must be provided for advection by auxiliary velocities");
  if (_dim == 3 && !getProblem().hasFunctor("az", /*thread_id=*/0))
    paramError("add_flow_equations",
               "Rhie Chow coefficient az must be provided for advection by auxiliary velocities");
}
