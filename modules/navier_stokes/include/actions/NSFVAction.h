//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

#include "MooseEnum.h"
#include "libmesh/fe_type.h"

/**
 * This class allows us to have a section of the input file like the
 * following which automatically adds variables, kernels, aux kernels, bcs
 * for setting up the incompressible/weakly-compressible Navier-Stokes equations.
 * Create it using the following input syntax:
 * [Modules]
 *   [NavierStokesFV]
 *     param_1 = value_1
 *     param_2 = value_2
 *     ...
 *   []
 * []
 */
class NSFVAction : public Action
{
public:
  static InputParameters validParams();

  NSFVAction(InputParameters parameters);

  virtual void act() override;

protected:
  /// Type that we use in Actions for declaring coupling between the solutions
  /// of different physics components
  typedef std::vector<VariableName> CoupledName;

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
  void addScalarSourceKernels();

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

  /// Add mixing length material for turbulence handling
  void addMixingLengthMaterial();

  /// Add boundary postprocessors for flux BCs
  void addBoundaryPostprocessors();

  /**
   * Add relationship manager to extend the number of ghosted layers if necessary.
   * This is executed before the kernels and other objects are added.
   */
  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

  /// Subdomains Navier-Stokes equation is defined on
  std::vector<SubdomainName> _blocks;

  /// Mesh dimension
  unsigned int _dim;

  /// Equation type, transient or steady-state
  const MooseEnum _type;
  /// Compressibility type, can be compressible, incompressible
  /// or weakly-compressible
  const MooseEnum _compressibility;
  // Switch that can be used to create an integrated energy equation for
  /// incompressible/weakly compressible simulations.
  const bool _has_energy_equation;
  /// Switch to use to enable the Boussinesq approximation for incompressible
  /// fluid simulations
  const bool _boussinesq_approximation;
  /// Turbulent diffusivity handling type (mixing-length, etc.)
  const MooseEnum _turbulence_handling;

  /// Switch to show if porous medium treatment is requested or not
  const bool _porous_medium_treatment;
  /// The name of the auxiliary variable for the porosity field
  const MooseFunctorName _porosity_name;
  /// Switch to enable friction correction for the porous medium momentum
  /// equations
  const bool _use_friction_correction;

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
  /// Velocity function names at velocity inlet boundaries
  const std::vector<FunctionName> _momentum_inlet_function;
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
  const std::vector<SubdomainName> _ambient_convection_blocks;
  /// The heat exchange coefficients for ambient convection
  const std::vector<MooseFunctorName> _ambient_convection_alpha;
  /// The ambient temperature
  const std::vector<MooseFunctorName> _ambient_temperature;

  /// Subdomains where we want to have volumetric friction
  const std::vector<SubdomainName> _friction_blocks;
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
  /// Name of the thermal conductivity material property
  const MooseFunctorName _thermal_conductivity_name;
  /// Name of the thermal expansion material property
  const MooseFunctorName _thermal_expansion_name;

  /// List of the names of the scalar variables which are transported using this
  /// action
  const std::vector<NonlinearVariableName> _passive_scalar_names;
  /// Initial values for the passive scalar fields
  const std::vector<Real> _initial_scalar_variable;
  /// Passive scalar diffusivities
  const std::vector<MooseFunctorName> _passive_scalar_diffusivity;
  /// Passive scalar source terms
  const std::vector<MooseFunctorName> _passive_scalar_source;
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

  /// If a two-term Taylor expansion is needed for the determination of the boundary values
  /// of the pressure
  const bool _pressure_two_term_bc_expansion;
  /// If a two-term Taylor expansion is needed for the determination of the boundary values
  /// of the velocity/momentum
  const bool _momentum_two_term_bc_expansion;
  /// If a two-term Taylor expansion is needed for the determination of the boundary values
  /// of the temperature/energy
  const bool _energy_two_term_bc_expansion;
  /// If a two-term Taylor expansion is needed for the determination of the boundary values
  /// of the passive scalar fields
  const bool _passive_scalar_two_term_bc_expansion;

  /// The scaling factor for the mass variables (for incompressible simulation this is pressure scaling)
  const Real _mass_scaling;
  /// The scaling factor for the momentum variables
  const Real _momentum_scaling;
  /// The scaling factor for the energy variables
  const Real _energy_scaling;
  /// The scaling factor for the passive scalar variables
  const Real _passive_scalar_scaling;

  /// List to show which advected scalar field variable needs to be created within
  /// this action
  std::vector<bool> _create_scalar_variable;

private:
  /// Process the mesh data and convert block names to block IDs
  void processBlocks();
  /// Process the supplied variable names and if they are not available, create them
  void processVariables();
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
};

template <typename T>
void
NSFVAction::checkBlockwiseConsistency(const std::string block_param_name,
                                      const std::vector<std::string> parameter_names)
{
  const std::vector<SubdomainName> & block_names =
      _pars.get<std::vector<SubdomainName>>(block_param_name);

  if (block_names.size())
  {
    for (const auto & block : block_names)
      if (std::find(_blocks.begin(), _blocks.end(), block) == _blocks.end())
        paramError(block_param_name,
                   "Block '" + block + "' is not present in the block IDs of the module!");

    for (const auto & param_name : parameter_names)
    {
      const std::vector<T> & param_vector = _pars.get<std::vector<T>>(param_name);
      if (block_names.size() != param_vector.size())
        paramError(param_name,
                   "The number of entries in '" + param_name +
                       "' is not the same as the number of blocks in '" + block_param_name + "'!");
    }
  }
  else
  {
    unsigned int previous_size = 0;
    for (unsigned int param_i = 0; param_i < parameter_names.size(); ++param_i)
    {
      const std::vector<T> & param_vector = _pars.get<std::vector<T>>(parameter_names[param_i]);
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
