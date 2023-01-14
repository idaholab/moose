//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppFieldTransfer.h"

namespace libMesh
{
class DofObject;
}

/**
 * Copy the fields directly from one application to another, based on degree-of-freedom indexing
 */
class MultiAppDofCopyTransfer : public MultiAppFieldTransfer
{
public:
  static InputParameters validParams();

  MultiAppDofCopyTransfer(const InputParameters & parameters);

  void initialSetup() override;

protected:
  /**
   * Performs the transfer of a variable between two problems if they have the same mesh.
   */
  void transfer(FEProblemBase & to_problem, FEProblemBase & from_problem);

  /**
   * Performs the transfer of values between a node or element.
   */
  void transferDofObject(libMesh::DofObject * to_object,
                         libMesh::DofObject * from_object,
                         MooseVariableFieldBase & to_var,
                         MooseVariableFieldBase & from_var,
                         NumericVector<Number> & to_solution,
                         NumericVector<Number> & from_solution);

  /// Whether block restriction is active
  const bool _has_block_restrictions;
  /// Subdomain IDs of the blocks to transfer from
  std::set<SubdomainID> _from_blocks;
  /// Subdomain IDs of the blocks to transfer to
  std::set<SubdomainID> _to_blocks;
};
