//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWACTIONBASE_H
#define POROUSFLOWACTIONBASE_H

#include "Action.h"
#include "PorousFlowDependencies.h"

class PorousFlowActionBase;

template <>
InputParameters validParams<PorousFlowActionBase>();

/**
 * Base class for PorousFlow actions.  This act() method simply
 * defines the name of the PorousFlowDictator.  However, this
 * class also contains a number of utility functions that may
 * be used by derived classes
 */
class PorousFlowActionBase : public Action, public PorousFlowDependencies
{
public:
  PorousFlowActionBase(const InputParameters & params);

  virtual void act() override;

protected:
  /**
   * List of Kernels, AuxKernels, Materials, etc, to be added.
   * This list will be used to determine what Materials need
   * to be added.
   * Actions may add or remove things from this list
   */
  std::vector<std::string> _objects_to_add;

  /// The name of the PorousFlowDictator object to be added
  const std::string _dictator_name;

  /// gravity
  const RealVectorValue _gravity;

  /// Name of the mass-fraction variables (if any)
  const std::vector<VariableName> & _mass_fraction_vars;

  /// Number of mass-fraction variables
  const unsigned _num_mass_fraction_vars;

  /// Name of the temperature variable (if any)
  const std::vector<VariableName> & _temperature_var;

  /// displacement NonlinearVariable names (if any)
  const std::vector<NonlinearVariableName> & _displacements;

  /// number of displacement variables supplied
  const unsigned _ndisp;

  /// displacement Variable names
  std::vector<VariableName> _coupled_displacements;

  /// Coordinate system of the simulation (eg RZ, XYZ, etc)
  Moose::CoordinateSystemType _coord_system;

  /**
   * add the PorousFlowDictator object
   */
  virtual void addDictator() = 0;

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
   * Adds a quadpoint volumetric strain material
   * @param displacements the names of the displacement variables
   * @param consistent_with_displaced_mesh The volumetric strain should be consistent with the
   * displaced mesh
   */
  void addVolumetricStrainMaterial(const std::vector<VariableName> & displacements,
                                   bool consistent_with_displaced_mesh);

  /**
   * Adds a single-component fluid Material
   * @param phase the phase number of the fluid
   * @param fp the name of the FluidProperties UserObject
   * @param compute_density_and_viscosity compute the density and viscosity of the fluid
   * @param compute_internal_energy compute the fluid internal energy
   * @param compute_enthalpy compute the fluid enthalpy
   * @param at_nodes add a nodal material
   */
  void addSingleComponentFluidMaterial(bool at_nodes,
                                       unsigned phase,
                                       bool compute_density_and_viscosity,
                                       bool compute_internal_energy,
                                       bool compute_enthalpy,
                                       const UserObjectName & fp);

  /**
   * Adds a brine fluid Material
   * @param xnacl the variable containing the mass fraction of NaCl in the fluid
   * @param phase the phase number of the fluid
   * @param compute_density_and_viscosity compute the density and viscosity of the fluid
   * @param compute_internal_energy compute the fluid internal energy
   * @param compute_enthalpy compute the fluid enthalpy
   * @param at_nodes add a nodal material
   */
  void addBrineMaterial(const VariableName xnacl,
                        bool at_nodes,
                        unsigned phase,
                        bool compute_density_and_viscosity,
                        bool compute_internal_energy,
                        bool compute_enthalpy);

  /**
   * Adds a relative-permeability Material of the Corey variety
   * @param phase the phase number of the fluid
   * @param n The Corey exponent
   * @param s_res The residual saturation for this phase
   * @param sum_s_res The sum of residual saturations over all phases
   */
  void
  addRelativePermeabilityCorey(bool at_nodes, unsigned phase, Real n, Real s_res, Real sum_s_res);

  /**
   * Adds a relative-permeability Material of the FLAC variety
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

  /**
   * Adds a PorousFlowJoiner for the material_property Material
   * @param at_nodes if true: the Joiner should produce a nodal material, otherwise: produce a QP
   * material
   * @param material_property Join this PorousFlow Material
   * @param output_name The name given to this PorousFlowJoiner in the input file
   */
  void
  addJoiner(bool at_nodes, const std::string & material_property, const std::string & output_name);

  /**
   * Adds a PorousFlowJoiner for the fluid density
   * @param at_nodes if true: the Joiner should produce a nodal material, otherwise: produce a QP
   * material
   */
  void joinDensity(bool at_nodes);

  /**
   * Adds a PorousFlowJoiner for the fluid viscosity
   * @param at_nodes if true: the Joiner should produce a nodal material, otherwise: produce a QP
   * material
   */
  void joinViscosity(bool at_nodes);

  /**
   * Adds a PorousFlowJoiner for the fluid relative permeability
   * @param at_nodes if true: the Joiner should produce a nodal material, otherwise: produce a QP
   * material
   */
  void joinRelativePermeability(bool at_nodes);

  /**
   * Adds a PorousFlowJoiner for the fluid internal energy
   * @param at_nodes if true: the Joiner should produce a nodal material, otherwise: produce a QP
   * material
   */
  void joinInternalEnergy(bool at_nodes);

  /**
   * Adds a PorousFlowJoiner for the fluid enthalpy
   * @param at_nodes if true: the Joiner should produce a nodal material, otherwise: produce a QP
   * material
   */
  void joinEnthalpy(bool at_nodes);
};

#endif // POROUSFLOWACTIONBASE_H
