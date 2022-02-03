//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseVariableBase.h"

// Forward declarations
class InputParameters;
class MooseObject;
class MooseApp;

/**
 * Interface for objects that need coupling capabilities
 *
 */
class LazyCoupleable
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if we need to couple with nodal values, otherwise false
   */
  LazyCoupleable(const MooseObject * moose_object);

  virtual ~LazyCoupleable() = default;

  /**
   * Sets the FEProblem pointer which can (and is expected) to happen long after this interface is
   * constructed. Once this pointer is set, this interface can retrieve the information it requires
   * from the underlying class and the internal methods can be called. Errors are throw if internal
   * methods are called when this pointer is nullptr.
   */
  void setFEProblemPtr(FEProblemBase * fe_problem);

private:
  void init();

  /**
   * Returns the index for a coupled variable by name
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Index of coupled variable, if this is an optionally coupled variable that wasn't
   * provided this will return a unique "invalid" index.
   */
  unsigned int & coupled(const std::string & var_name, unsigned int comp = 0);

  // Reference to the interface's input parameters
  const InputParameters & _l_parameters;

  /// The name of the object this interface is part of
  const std::string & _l_name;

  /// Reference to FEProblemBase
  FEProblemBase * _l_fe_problem;

  /// Reference to the MooseApp
  MooseApp & _l_app;

  /// Coupled vars whose values we provide
  std::map<std::string, std::unique_ptr<unsigned int>> _coupled_var_numbers;
};
