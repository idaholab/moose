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

#ifndef SCALARCOUPLEABLE_H
#define SCALARCOUPLEABLE_H

#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "InputParameters.h"

/**
 * Interface for objects that needs scakar coupling capabilities
 *
 */
class ScalarCoupleable
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   */
  ScalarCoupleable(InputParameters & parameters);

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
  virtual VariableValue & coupledScalarValueOld(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns the older (two time steps previous) value of a scalar coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a older VariableValue for the coupled variable
   */
  virtual VariableValue & coupledScalarDot(const std::string & var_name, unsigned int comp = 0);

  /**
   * Time derivative of a scalar coupled variable with respect to the coefficients
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable with respect to the coefficients
   */
  virtual VariableValue & coupledScalarDotDu(const std::string & var_name, unsigned int comp = 0);

protected:
  /// Coupled vars whose values we provide
  std::map<std::string, std::vector<MooseVariableScalar *> > _coupled_scalar_vars;

  /// Vector of coupled variables
  std::vector<MooseVariableScalar *> _coupled_moose_scalar_vars;

  /// True if implicit value is required
  bool _sc_is_implicit;

  /**
   * Extract pointer to a scalar coupled variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  MooseVariableScalar *getScalarVar(const std::string & var_name, unsigned int comp);
};

#endif // SCALARCOUPLEABLE_H
