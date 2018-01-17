/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef LAZYCOUPLEABLE_H
#define LAZYCOUPLEABLE_H

// MOOSE includes
#include "MooseVariableBase.h"

// Forward declarations
class InputParameters;
class MooseObject;

/**
 * Interface for objects that needs coupling capabilities
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

  ~LazyCoupleable();

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

#endif /* LAZYCOUPLEABLE_H */
