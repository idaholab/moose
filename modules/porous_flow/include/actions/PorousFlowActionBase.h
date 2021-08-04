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
#include "PorousFlowDependencies.h"
#include "MooseEnum.h"

#include "libmesh/vector_value.h"

/**
 * Base class for PorousFlow actions.  This act() method makes consistency checks and
 * calls several methods that should be implemented in derived classes. This class also
 * contains a number of utility functions that may be used by derived classes.
 *
 * Derived classes should typically only override the following methods:
 * * addUserObjects()
 * * addMaterialDependencies()
 * * addMaterials()
 * * addAuxObjects()
 * * addKernels()
 */
class PorousFlowActionBase : public Action, public PorousFlowDependencies
{
public:
  static InputParameters validParams();

  PorousFlowActionBase(const InputParameters & params);

  virtual void act() override;

  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType when_type) override;

protected:
  /**
   * List of Kernels, AuxKernels, Materials, etc, that are added in this input file.
   * This list will be used to determine what Materials need
   * to be added. Actions may add or remove things from this list
   */
  std::vector<std::string> _included_objects;

  /// The name of the PorousFlowDictator object to be added
  const std::string _dictator_name;

  /// Number of aqueous-equilibrium secondary species
  const unsigned int _num_aqueous_equilibrium;

  /// Number of aqeuous-kinetic secondary species that are involved in mineralisation
  const unsigned int _num_aqueous_kinetic;

  /// Gravity
  const RealVectorValue _gravity;

  /// Name of the mass-fraction variables (if any)
  const std::vector<VariableName> & _mass_fraction_vars;

  /// Number of mass-fraction variables
  const unsigned _num_mass_fraction_vars;

  /// Name of the temperature variable (if any)
  const std::vector<VariableName> & _temperature_var;

  /// Displacement NonlinearVariable names (if any)
  const std::vector<VariableName> & _displacements;

  /// Number of displacement variables supplied
  const unsigned _ndisp;

  /// Displacement Variable names
  std::vector<VariableName> _coupled_displacements;

  /// Flux limiter type in the Kuzmin-Turek FEM-TVD stabilization scheme
  const MooseEnum _flux_limiter_type;

  const enum class StabilizationEnum { None, Full, KT } _stabilization;

  /// Evaluate strain at the nearest quadpoint for porosity that depends on strain
  const bool _strain_at_nearest_qp;

  /// Coordinate system of the simulation (eg RZ, XYZ, etc)
  Moose::CoordinateSystemType _coord_system;

  /// Flag to denote if the simulation is transient
  bool _transient;

  /**
   * Add the PorousFlowDictator object
   */
  virtual void addDictator() = 0;

  /**
   * Add all other UserObjects
   */
  virtual void addUserObjects();

  /**
   * Add all AuxVariables and AuxKernels
   */
  virtual void addAuxObjects();

  /**
   * Add all Kernels
   */
  virtual void addKernels();

  /**
   * Add all material dependencies so that the correct
   * version of each material can be added
   */
  virtual void addMaterialDependencies();

  /**
   * Add all Materials
   */
  virtual void addMaterials();

  /**
   * Add an AuxVariable and AuxKernel to calculate saturation
   * @param phase Saturation for this fluid phase
   */
  void addSaturationAux(unsigned phase);

  /**
   * Add AuxVariables and AuxKernels to calculate Darcy velocity.
   * @param gravity gravitational acceleration pointing downwards (eg (0, 0, -9.8))
   */
  void addDarcyAux(const RealVectorValue & gravity);

  /**
   * Add AuxVariables and AuxKernels to compute effective stress.
   */
  void addStressAux();

  /**
   * Adds a nodal and a quadpoint Temperature material
   * @param at_nodes Add nodal temperature Material
   */
  void addTemperatureMaterial(bool at_nodes);

  /**
   * Adds a nodal and a quadpoint MassFraction material
   * @param at_nodes Add nodal mass-fraction material
   */
  void addMassFractionMaterial(bool at_nodes);

  /**
   * Adds a nodal and a quadpoint effective fluid pressure material
   * @param at_nodes Add nodal effective fluid pressure material
   */
  void addEffectiveFluidPressureMaterial(bool at_nodes);

  /**
   * Adds a PorousFlowNearestQp material
   */
  void addNearestQpMaterial();

  /**
   * Adds a quadpoint volumetric strain material
   * @param displacements the names of the displacement variables
   * @param base_name The base_name used in the TensorMechanics strain calculator
   * displaced mesh
   */
  void addVolumetricStrainMaterial(const std::vector<VariableName> & displacements,
                                   const std::string & base_name);

  /**
   * Adds a single-component fluid Material
   * @param phase the phase number of the fluid
   * @param fp the name of the FluidProperties UserObject
   * @param compute_density_and_viscosity compute the density and viscosity of the fluid
   * @param compute_internal_energy compute the fluid internal energy
   * @param compute_enthalpy compute the fluid enthalpy
   * @param at_nodes add a nodal material
   * @param temperature_unit the unit of temperature (Kelvin or Celsius)
   * @param pressure_unit the unit of pressure (MPa or Pa)
   * @param time_unit the unit of time (seconds, days, hours, etc)
   */
  void addSingleComponentFluidMaterial(bool at_nodes,
                                       unsigned phase,
                                       bool compute_density_and_viscosity,
                                       bool compute_internal_energy,
                                       bool compute_enthalpy,
                                       const UserObjectName & fp,
                                       const MooseEnum & temperature_unit,
                                       const MooseEnum & pressure_unit,
                                       const MooseEnum & time_unit);

  /**
   * Adds a brine fluid Material
   * @param xnacl the variable containing the mass fraction of NaCl in the fluid
   * @param phase the phase number of the fluid
   * @param compute_density_and_viscosity compute the density and viscosity of the fluid
   * @param compute_internal_energy compute the fluid internal energy
   * @param compute_enthalpy compute the fluid enthalpy
   * @param at_nodes add a nodal material
   * @param temperature_unit the unit of temperature (Kelvin or Celsius)
   */
  void addBrineMaterial(const VariableName xnacl,
                        bool at_nodes,
                        unsigned phase,
                        bool compute_density_and_viscosity,
                        bool compute_internal_energy,
                        bool compute_enthalpy,
                        const MooseEnum & temperature_unit);

  /**
   * Adds a relative-permeability Material of the constant variety (primarily to
   * add kr = 1 in actions that add a default relatively permeability for objects
   * that require kr even when the flow is fully saturated with a single phase)
   * @param at_nodes whether the material is nodal
   * @param phase the phase number of the fluid
   * @param kr the relative permeability
   */
  void addRelativePermeabilityConst(bool at_nodes, unsigned phase, Real kr);

  /**
   * Adds a relative-permeability Material of the Corey variety
   * @param at_nodes whether the material is nodal
   * @param phase the phase number of the fluid
   * @param n The Corey exponent
   * @param s_res The residual saturation for this phase
   * @param sum_s_res The sum of residual saturations over all phases
   */
  void
  addRelativePermeabilityCorey(bool at_nodes, unsigned phase, Real n, Real s_res, Real sum_s_res);

  /**
   * Adds a relative-permeability Material of the FLAC variety
   * @param at_nodes whether the material is nodal
   * @param phase the phase number of the fluid
   * @param m The FLAC exponent
   * @param s_res The residual saturation for this phase
   * @param sum_s_res The sum of residual saturations over all phases
   */
  void
  addRelativePermeabilityFLAC(bool at_nodes, unsigned phase, Real m, Real s_res, Real sum_s_res);

  /**
   * Adds a van Genuchten capillary pressure UserObject
   * @param m van Genuchten exponent
   * @param alpha van Genuchten alpha
   * @param userobject_name name of the user object
   */
  void addCapillaryPressureVG(Real m, Real alpha, std::string userobject_name);

  void addAdvectiveFluxCalculatorSaturated(unsigned phase,
                                           bool multiply_by_density,
                                           std::string userobject_name);

  void addAdvectiveFluxCalculatorUnsaturated(unsigned phase,
                                             bool multiply_by_density,
                                             std::string userobject_name);

  void addAdvectiveFluxCalculatorSaturatedHeat(unsigned phase,
                                               bool multiply_by_density,
                                               std::string userobject_name);

  void addAdvectiveFluxCalculatorUnsaturatedHeat(unsigned phase,
                                                 bool multiply_by_density,
                                                 std::string userobject_name);

  void addAdvectiveFluxCalculatorSaturatedMultiComponent(unsigned phase,
                                                         unsigned fluid_component,
                                                         bool multiply_by_density,
                                                         std::string userobject_name);

  void addAdvectiveFluxCalculatorUnsaturatedMultiComponent(unsigned phase,
                                                           unsigned fluid_component,
                                                           bool multiply_by_density,
                                                           std::string userobject_name);
};
