//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

private:
  /// For a node, finds an element we can use to evaluate the (continuous) source variable
  const Elem * elemOnNodeVariableIsDefinedOn() const;
};
