//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"

// MOOSE includes
#include "MooseVariableBase.h"

// C++ includes
#include <unordered_map>
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
   * Get the list of coupled scalar variables
   * @return The list of coupled variables
   */
  const std::vector<MooseVariableScalar *> & getCoupledMooseScalarVars()
  {
    return _coupled_moose_scalar_vars;
  }

  const std::set<TagID> & getScalarVariableCoupleableVectorTags() const
  {
    return _sc_coupleable_vector_tags;
  }

  const std::set<TagID> & getScalarVariableCoupleableMatrixTags() const
  {
    return _sc_coupleable_matrix_tags;
  }

protected:
  /**
   * Returns true if a variables has been coupled_as name.
   * @param var_name The of the coupled variable
   * @param i By default 0, in general the index to test in a vector of MooseVariable pointers.
   */
  bool isCoupledScalar(const std::string & var_name, unsigned int i = 0) const;

  /**
   * Return the number of components to the coupled scalar variable
   * @param var_name The of the coupled variable
   */
  unsigned int coupledScalarComponents(const std::string & var_name) const;

  /**
   * Returns the index for a scalar coupled variable by name
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Index of coupled variable
   */
  unsigned int coupledScalar(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Returns the order for a scalar coupled variable by name
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Order of coupled variable
   */
  Order coupledScalarOrder(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Returns value of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  const VariableValue & coupledScalarValue(const std::string & var_name,
                                           unsigned int comp = 0) const;

  /**
   * Returns AD value of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a ADVariableValue for the coupled variable
   */
  const ADVariableValue & adCoupledScalarValue(const std::string & var_name,
                                               unsigned int comp = 0) const;

  /**
   * Returns value of a coupled scalar variable for use in templated automatic differentiation
   * classes
   * @param var_name Name of coupled scalar variable
   * @param comp Component number for vector of coupled scalar variables
   * @return Reference to a GenericVariableValue for the coupled scalar variable
   */
  template <bool is_ad>
  const GenericVariableValue<is_ad> & coupledGenericScalarValue(const std::string & var_name,
                                                                unsigned int comp = 0) const;

  /**
   * Returns value of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param tag Tag ID of coupled vector ;
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  const VariableValue &
  coupledVectorTagScalarValue(const std::string & var_name, TagID tag, unsigned int comp = 0) const;

  /**
   * Returns value of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param tag Tag ID of coupled matrix;
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   */
  const VariableValue &
  coupledMatrixTagScalarValue(const std::string & var_name, TagID tag, unsigned int comp = 0) const;

  /**
   * Returns the old (previous time step) value of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a old VariableValue for the coupled variable
   */
  const VariableValue & coupledScalarValueOld(const std::string & var_name,
                                              unsigned int comp = 0) const;

  /**
   * Returns the older (two time steps previous) value of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a older VariableValue for the coupled variable
   */
  const VariableValue & coupledScalarValueOlder(const std::string & var_name,
                                                unsigned int comp = 0) const;
  /**
   * Returns the time derivative of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a time derivative VariableValue for the coupled variable
   */
  const VariableValue & coupledScalarDot(const std::string & var_name, unsigned int comp = 0) const;

  /**
   * Returns the second time derivative of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a time derivative VariableValue for the coupled variable
   */
  const VariableValue & coupledScalarDotDot(const std::string & var_name,
                                            unsigned int comp = 0) const;

  /**
   * Returns the old time derivative of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a time derivative VariableValue for the coupled variable
   */
  const VariableValue & coupledScalarDotOld(const std::string & var_name,
                                            unsigned int comp = 0) const;

  /**
   * Returns the old second time derivative of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a time derivative VariableValue for the coupled variable
   */
  const VariableValue & coupledScalarDotDotOld(const std::string & var_name,
                                               unsigned int comp = 0) const;

  /**
   * Time derivative of a scalar coupled variable with respect to the coefficients
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * with respect to the coefficients
   */
  const VariableValue & coupledScalarDotDu(const std::string & var_name,
                                           unsigned int comp = 0) const;

  /**
   * Second time derivative of a scalar coupled variable with respect to the coefficients
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * with respect to the coefficients
   */
  const VariableValue & coupledScalarDotDotDu(const std::string & var_name,
                                              unsigned int comp = 0) const;

  /**
   * Extract pointer to a scalar coupled variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  const MooseVariableScalar * getScalarVar(const std::string & var_name, unsigned int comp) const;

protected:
  // Reference to FEProblemBase
  FEProblemBase & _sc_fe_problem;

  /// Thread ID of the thread using this object
  const THREAD_ID _sc_tid;

  /// Scalar zero
  const Real & _real_zero;

  /// Zero value of a scalar variable
  const VariableValue & _scalar_zero;

  /// Zero point
  const Point & _point_zero;

private:
  /**
   * Helper method to return (and insert if necessary) the default value
   * for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default value
   * @return VariableValue * a pointer to the associated VarirableValue.
   */
  const VariableValue * getDefaultValue(const std::string & var_name) const;

  /**
   * Helper method to return (and insert if necessary) the AD default value
   * for an uncoupled variable.
   * @param var_name the name of the variable for which to retrieve a default value
   * @return ADVariableValue * a pointer to the associated ADVariableValue.
   */
  const ADVariableValue * getADDefaultValue(const std::string & var_name) const;

  /**
   * Check that the right kind of variable is being coupled in
   *
   * @param var_name The name of the coupled variable
   */
  void checkVar(const std::string & var_name) const;

  /**
   * Checks to make sure that the current Executioner has set "_is_transient" when old/older values
   * are coupled in.
   * @param name the name of the variable
   * @param fn_name The name of the function that called this method - used in the error message
   */
  void validateExecutionerType(const std::string & name, const std::string & fn_name) const;

  // Reference to the interface's input parameters
  const InputParameters & _sc_parameters;

  /// The name of the object this interface is part of
  const std::string & _sc_name;

  /// True if implicit value is required
  const bool _sc_is_implicit;

  /// Coupled vars whose values we provide
  std::unordered_map<std::string, std::vector<MooseVariableScalar *>> _coupled_scalar_vars;

  /// Will hold the default value for optional coupled scalar variables.
  mutable std::unordered_map<std::string, std::unique_ptr<VariableValue>> _default_value;

  /// Will hold the default AD value for optional coupled scalar variables.
  mutable std::unordered_map<std::string, std::unique_ptr<ADVariableValue>> _dual_default_value;

  /// Vector of coupled variables
  std::vector<MooseVariableScalar *> _coupled_moose_scalar_vars;

  /// Field variables coupled into this object (for error checking)
  std::unordered_map<std::string, std::vector<MooseVariableFieldBase *>> _sc_coupled_vars;

  /// The scalar coupleable vector tags
  mutable std::set<TagID> _sc_coupleable_vector_tags;
  /// The scalar coupleable matrix tags
  mutable std::set<TagID> _sc_coupleable_matrix_tags;
};
