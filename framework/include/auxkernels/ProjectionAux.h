//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

namespace libMesh
{
class System;
}

/**
 * Projects from one variable to another
 */
class ProjectionAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ProjectionAux(const InputParameters & parameters);

protected:
  enum ElemToNodeProjectionWeighting
  {
    VOLUME = 0,
    IDENTITY
  };

  virtual Real computeValue() override;

  /// The variable to project from
  const VariableValue & _v;

  /// A reference to the variable to project from
  /// We must use a field variable to support finite volume variables
  const MooseVariableFieldBase & _source_variable;

  /// The system owning the source variable
  const libMesh::System & _source_sys;

  /// Whether to use the auxkernel block restriction to limit the values for the source variables
  bool _use_block_restriction_for_source;

  /// How to weight element to node projections
  const ElemToNodeProjectionWeighting _elem_to_node_projection_weighting;

private:
  /// For a node, finds an element we can use to evaluate the (continuous) source variable
  const Elem * elemOnNodeVariableIsDefinedOn() const;

  /// Set for holding element dimensions when mapping from multiple element values to a node. We use
  /// this for error checking that we aren't doing volume weighted averaging with multiple element
  /// dimensions which would result in inconsistent units and consequently illogical results. We use
  /// this data member to reduce dynamic heap allocations
  std::unordered_set<unsigned short> _elem_dims;
};
