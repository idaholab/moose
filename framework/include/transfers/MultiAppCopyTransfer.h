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
class MultiAppCopyTransfer : public MultiAppFieldTransfer
{
public:
  static InputParameters validParams();

  MultiAppCopyTransfer(const InputParameters & parameters);

  void initialSetup() override;

  /**
   * Performs the transfer of a variable (Nonlinear or Auxiliary) to/from the Multiapp.
   */
  virtual void execute() override;

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

  virtual std::vector<VariableName> getFromVarNames() const override { return _from_var_names; }
  virtual std::vector<AuxVariableName> getToVarNames() const override { return _to_var_names; }

  // These attributes are used if a derived class supports transferring multiple variables
  /// Name of variables transferring from
  const std::vector<VariableName> _from_var_names;
  /// Name of variables transferring to
  const std::vector<AuxVariableName> _to_var_names;

  // These attributes are used if a derived class only supports one variable
  /// Name of variables transferring from
  VariableName _from_var_name;
  /// Name of variables transferring to
  AuxVariableName _to_var_name;
};
