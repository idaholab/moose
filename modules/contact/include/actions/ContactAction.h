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
#include "MooseTypes.h"
#include "MooseEnum.h"

enum class ContactModel
{
  FRICTIONLESS,
  GLUED,
  COULOMB,
};

enum class ContactFormulation
{
  RANFS,
  KINEMATIC,
  PENALTY,
  AUGMENTED_LAGRANGE,
  TANGENTIAL_PENALTY,
  MORTAR,
  MORTAR_PENALTY
};

/**
 * Action class for creating constraints, kernels, and user objects necessary for mechanical
 * contact.
 */
class ContactAction : public Action
{
public:
  static InputParameters validParams();

  ContactAction(const InputParameters & params);

  virtual void act() override;

  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

  /**
   * Get contact model
   * @return enum
   */
  static MooseEnum getModelEnum();

  /**
   * Get contact formulation
   * @return enum
   */

  static MooseEnum getFormulationEnum();
  /**
   * Get contact system
   * @return enum
   */
  static MooseEnum getSystemEnum();

  /**
   * Get smoothing type
   * @return enum
   */
  static MooseEnum getSmoothingEnum();

  /**
   * Define parameters used by multiple contact objects
   * @return InputParameters object populated with common parameters
   */
  static InputParameters commonParameters();

protected:
  /// Primary/Secondary boundary name pairs for mechanical contact
  std::vector<std::pair<BoundaryName, BoundaryName>> _boundary_pairs;

  /// List of all possible boundaries for contact for automatic pairing (optional)
  std::vector<BoundaryName> _automatic_pairing_boundaries;

  /// Contact model type enum
  const ContactModel _model;

  /// Contact formulation
  const ContactFormulation _formulation;

  /// Whether to use the dual Mortar approach
  bool _use_dual;

  /// Whether to generate the mortar mesh (useful in a restart simulation e.g.).
  const bool _generate_mortar_mesh;

  /// Whether mortar dynamic contact constraints are to be used
  const bool _mortar_dynamics;

  /// Type that we use in Actions for declaring coupling
  typedef std::vector<VariableName> CoupledName;

private:
  /**
   * Generate mesh and other Moose objects for Mortar contact
   */
  void addMortarContact();
  /**
   * Generate constraints for node to face contact
   */
  void addNodeFaceContact();
  /**
   * Add single contact pressure auxiliary kernel for various contact action objects
   */
  void addContactPressureAuxKernel();
  /**
   * Remove repeated contact pairs from _boundary_pairs.
   */
  void removeRepeatedPairs();
  /**
   * Create contact pairs between all boundaries whose centroids are within a user-specified
   * distance of each other.
   */
  void createSidesetPairsFromGeometry();
};
