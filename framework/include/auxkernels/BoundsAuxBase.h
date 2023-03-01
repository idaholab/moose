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
/**
 * This is a base class used to set an upper and/or lower bound of a variable
 * for the PETSc's variational inequalities solver
 */
class BoundsAuxBase : public AuxKernel
{
public:
  static InputParameters validParams();

  enum BoundType
  {
    UPPER,
    LOWER
  };

  BoundsAuxBase(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /**
   * Method to get bound value for a variable.
   */
  virtual Real getBound() = 0;

  /// The bound type ("upper" or "lower")
  BoundType _type;

  /// Reference to the bounded vector of nonlinear system
  NumericVector<Number> & _bounded_vector;

  /// MOOSE variable (base class) we set the bound for
  MooseVariableFieldBase & _bounded_var;

  /// Name of MOOSE variable we set the bound for
  NonlinearVariableName _bounded_var_name;

  /// Pointer to the finite element variable we set the bound for. Will be null for finite volume
  MooseVariableFE<Real> * _fe_var;

private:
  /// Return the current DOF index to apply the bound on
  dof_id_type getDoFIndex() const;
};
