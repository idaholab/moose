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

enum class ExplicitDynamicsContactModel
{
  FRICTIONLESS,
  FRICTIONLESS_BALANCE
};

/**
 * Action class for creating constraints, kernels, and user objects necessary for mechanical
 * contact.
 */
class ExplicitDynamicsContactAction : public Action

{
public:
  static InputParameters validParams();

  ExplicitDynamicsContactAction(const InputParameters & params);

  virtual void act() override;

  /**
   * Get contact model
   * @return enum
   */
  static MooseEnum getModelEnum();

  /**
   * Define parameters used by multiple contact objects
   * @return InputParameters object populated with common parameters
   */
  static InputParameters commonParameters();

protected:
  /// Primary/Secondary boundary name pairs for mechanical contact
  std::vector<std::pair<BoundaryName, BoundaryName>> _boundary_pairs;

  /// Contact model type enum
  const ExplicitDynamicsContactModel _model;

  /// Type that we use in Actions for declaring coupling
  typedef std::vector<VariableName> CoupledName;

private:
  /**
   * Generate constraints for node to face contact
   */
  void addNodeFaceContact();
  /**
   * Add single contact pressure auxiliary kernel for various contact action objects
   */
  void addContactPressureAuxKernel();
};
