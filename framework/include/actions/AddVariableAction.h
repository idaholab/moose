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

#ifndef ADDVARIABLEACTION_H
#define ADDVARIABLEACTION_H

// MOOSE includes
#include "Action.h"

// libMesh includes
#include "libmesh/fe.h"

// Forward declerations
class AddVariableAction;

template<>
InputParameters validParams<AddVariableAction>();

/**
 * Adds nonlinear variable
 */
class AddVariableAction : public Action
{
public:

  /**
   * Class constructor
   */
  AddVariableAction(const std::string & name, InputParameters params);

  /**
   * Adds the nonlinear variable
   */
  virtual void act();

  /**
   * Get the possible variable families
   * @return MooseEnum with the possible variable families (e.g., LAGRANGE)
   */
  static MooseEnum getNonlinearVariableFamilies();

  /**
   * Get the possible variable orders
   * @return MooseEnum with the possible variable orders (e.g., SECOND)
   */
  static MooseEnum getNonlinearVariableOrders();

protected:

  /**
   * Create initial condition object
   *
   * If the user supplies a value for 'initial_condition' in the input file this
   * method will create the proper InitialCondition object.
   */
  void setInitialCondition();

  /**
   * Get the block ids from the input parameters
   * @return A set of block ids defined in the input
   */
  std::set<SubdomainID> getSubdomainIDs();

  /// FEType for the variable being created
  FEType _fe_type;

  /// True if the variable being created is a scalar
  bool _scalar_var;

  /// Absolute zero tolerance
  static const Real _abs_zero_tol;


  // std::string _variable_to_read;
};

#endif // ADDVARIABLEACTION_H
