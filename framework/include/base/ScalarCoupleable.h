//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SCALARCOUPLEABLE_H
#define SCALARCOUPLEABLE_H

#include "Moose.h"

// MOOSE includes
#include "MooseVariableBase.h"

// C++ includes
#include <map>
#include <string>
#include <vector>

// Forward declarations
class FEProblemBase;
class InputParameters;
class MooseObject;
class MooseVariableScalar;

/**
 * Interface for objects that needs scalar coupling capabilities
 *
 */
class ScalarCoupleable
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   */
  ScalarCoupleable(const MooseObject * moose_object);

  /**
   * Destructor for object
   */
  virtual ~ScalarCoupleable();

  /**
   * Get the list of coupled scalar variables
   * @return The list of coupled variables
   */
  const std::vector<MooseVariableScalar *> & getCoupledMooseScalarVars();

protected:
  // Reference to the interface's input parameters
  const InputParameters & _sc_parameters;

  /// The name of the object this interface is part of
  const std::string & _sc_name;

  /**
   * Returns true if a variables has been coupled_as name.
   * @param var_name The of the coupled variable
   * @param i By default 0, in general the index to test in a vector of MooseVariable pointers.
   */
  virtual bool isCoupledScalar(const std::string & var_name, unsigned int i = 0);

  /**
   * Return the number of components to the coupled scalar variable
   * @param var_name The of the coupled variable
   */
  virtual unsigned int coupledScalarComponents(const std::string & var_name);

  /**
   * Returns the index for a scalar coupled variable by name
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Index of coupled variable
   */
  virtual unsigned int coupledScalar(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns the order for a scalar coupled variable by name
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Order of coupled variable
   */
  virtual Order coupledScalarOrder(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns value of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  virtual VariableValue & coupledScalarValue(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns the old (previous time step) value of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a old VariableValue for the coupled variable
   */
  virtual VariableValue & coupledScalarValueOld(const std::string & var_name,
                                                unsigned int comp = 0);

  /**
   * Returns the older (two time steps previous) value of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a older VariableValue for the coupled variable
   */
  virtual VariableValue & coupledScalarValueOlder(const std::string & var_name,
                                                  unsigned int comp = 0);
  /**
   * Returns the time derivative of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a time derivative VariableValue for the coupled variable
   */
  virtual VariableValue & coupledScalarDot(const std::string & var_name, unsigned int comp = 0);

  /**
   * Time derivative of a scalar coupled variable with respect to the coefficients
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * with respect to the coefficients
   */
  virtual VariableValue & coupledScalarDotDu(const std::string & var_name, unsigned int comp = 0);

protected:
  // Reference to FEProblemBase
  FEProblemBase & _sc_fe_problem;

  /// Coupled vars whose values we provide
  std::map<std::string, std::vector<MooseVariableScalar *>> _coupled_scalar_vars;

  /// Will hold the default value for optional coupled scalar variables.
  std::map<std::string, VariableValue *> _default_value;

  /// Vector of coupled variables
  std::vector<MooseVariableScalar *> _coupled_moose_scalar_vars;

  /// True if implicit value is required
  bool _sc_is_implicit;

  /// Local InputParameters
  const InputParameters & _coupleable_params;

  /// Thread ID of the thread using this object
  THREAD_ID _sc_tid;

  /// Scalar zero
  const Real & _real_zero;

  /// Zero value of a scalar variable
  const VariableValue & _scalar_zero;

  /// Zero point
  const Point & _point_zero;

  /**
   * Helper method to return (and insert if necessary) the default value
   * for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default value
   * @return VariableValue * a pointer to the associated VarirableValue.
   */
  VariableValue * getDefaultValue(const std::string & var_name);

  /**
   * Check that the right kind of variable is being coupled in
   *
   * @param var_name The name of the coupled variable
   */
  void checkVar(const std::string & var_name);

  /**
   * Extract pointer to a scalar coupled variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  MooseVariableScalar * getScalarVar(const std::string & var_name, unsigned int comp);

private:
  /// Field variables coupled into this object (for error checking)
  std::map<std::string, std::vector<MooseVariable *>> _sc_coupled_vars;
};

#endif // SCALARCOUPLEABLE_H
