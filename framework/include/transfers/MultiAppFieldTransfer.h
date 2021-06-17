//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppFieldTransfer;
class MooseVariableFieldBase;
namespace libMesh
{
class DofObject;
}

template <>
InputParameters validParams<MultiAppFieldTransfer>();

/**
 *  intermediary class that allows variable names as inputs
 */
class MultiAppFieldTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppFieldTransfer(const InputParameters & parameters);

  virtual void initialSetup();

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

  /// Virtual function defining variables to be transferred
  virtual std::vector<VariableName> getFromVarNames() const = 0;
  /// Virtual function defining variables to transfer to
  virtual std::vector<AuxVariableName> getToVarNames() const = 0;
};
