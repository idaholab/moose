//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Output.h"

// Forward declarations
class NonlinearSystemBase;
/**
 * A class for producing various debug related outputs
 *
 * This class may be used from inside the [Outputs] block or via the [Debug] block (preferred)
 */
class BlockRestrictionDebugOutput : public Output
{
public:
  static InputParameters validParams();

  /**
   * Get the supported scopes of output (e.g., variables, etc.)
   */
  static MultiMooseEnum getScopes(std::string default_scopes = "");

  BlockRestrictionDebugOutput(const InputParameters & parameters);

protected:
  /// multi-enum of object types to show the block-restriction for
  const MultiMooseEnum & _scope;

  /// Reference to MOOSE's nonlinear system
  const NonlinearSystemBase & _nl;

  /// Reference to libMesh system
  const libMesh::System & _sys;

  /**
   * Perform the debugging output
   */
  virtual void output() override;

  /**
   * Prints block-restriction information
   */
  void printBlockRestrictionMap() const;

  /**
   * Prints object groups with identical block restrictions
   */
  void printBlockRestrictionGroups() const;

  /**
   * Prints object groups with identical boundary restrictions
   */
  void printBoundaryRestrictionGroups() const;

  /// Whether to print the existing per-block restriction map
  const bool & _show_block_restriction_map;

  /// Whether to print object groups by identical block restriction
  const bool & _show_block_restriction_groups;

  /// Whether to print object groups by identical boundary restriction
  const bool & _show_boundary_restriction_groups;
};
