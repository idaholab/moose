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

#ifndef COUPLEABLE_H
#define COUPLEABLE_H

#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "InputParameters.h"


/**
 * Interface for objects that needs coupling capabilities
 *
 */
class Coupleable
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if we need to couple with nodal values, otherwise false
   */
  Coupleable(InputParameters & parameters, bool nodal);

  /**
   * Destructor for object
   */
  virtual ~Coupleable();

  /**
   * Get the list of coupled variables
   * @return The list of coupled variables
   */
  const std::map<std::string, std::vector<MooseVariable *> > & getCoupledVars() { return _coupled_vars; }

  /**
   * Get the list of coupled variables
   * @return The list of coupled variables
   */
  const std::vector<MooseVariable *> & getCoupledMooseVars() { return _coupled_moose_vars; }

protected:
  /**
   * Returns true if a variables has been coupled as name.
   * @param var_name The name the kernel wants to refer to the variable as.
   * @return True if a coupled variable has the supplied name
   */
  virtual bool isCoupled(const std::string & var_name, unsigned int i = 0);

  /**
   * Number of coupled components
   * @param var_name Name of the variable
   * @return number of components this variable has (usually 1)
   */
  unsigned int coupledComponents(const std::string & var_name);

  virtual void coupledCallback(const std::string & var_name, bool is_old);

  /**
   * Returns the index for a coupled variable by name
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Index of coupled variable
   */
  virtual unsigned int coupled(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns value of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue for the coupled variable
   * @see Kernel::value
   */
  virtual VariableValue & coupledValue(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old value from previous time step  of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the old value of the coupled variable
   * @see Kernel::valueOld
   */
  virtual VariableValue & coupledValueOld(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old value from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the older value of the coupled variable
   * @see Kernel::valueOlder
   */
  virtual VariableValue & coupledValueOlder(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns gradient of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the gradient of the coupled variable
   * @see Kernel::gradient
   */
  virtual VariableGradient & coupledGradient(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old gradient from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the old gradient of the coupled variable
   * @see Kernel::gradientOld
   */
  virtual VariableGradient & coupledGradientOld(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old gradient from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableGradient containing the older gradient of the coupled variable
   * @see Kernel::gradientOlder
   */
  virtual VariableGradient & coupledGradientOlder(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns second derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the second derivative of the coupled variable
   * @see Kernel::second
   */
  virtual VariableSecond & coupledSecond(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old second derivative from previous time step of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the old second derivative of the coupled variable
   * @see Kernel::secondOld
   */
  virtual VariableSecond & coupledSecondOld(const std::string & var_name, unsigned int comp = 0);

  /**
   * Returns an old second derivative from two time steps previous of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableSecond containing the older second derivative of the coupled variable
   * @see Kernel::secondOlder
   */
  virtual VariableSecond & coupledSecondOlder(const std::string & var_name, unsigned int comp = 0);

  /**
   * Time derivative of a coupled variable
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable
   * @see Kernel::dot
   */
  virtual VariableValue & coupledDot(const std::string & var_name, unsigned int comp = 0);

  /**
   * Time derivative of a coupled variable with respect to the coefficients
   * @param var_name Name of coupled variable
   * @param comp Component number for vector of coupled variables
   * @return Reference to a VariableValue containing the time derivative of the coupled variable with respect to the coefficients
   * @see Kernel:dotDu
   */
  virtual VariableValue & coupledDotDu(const std::string & var_name, unsigned int comp = 0);

protected:
  std::map<std::string, std::vector<MooseVariable *> > _coupled_vars; ///< Coupled vars whose values we provide
  std::vector<MooseVariable *> _coupled_moose_vars; ///< Vector of coupled variables
  bool _nodal; ///< True if we provide coupling to nodal values
  bool _c_is_implicit; ///< True if implicit value is required

  /**
   * Extract pointer to a coupled variable
   * @param var_name Name of parameter desired
   * @param comp Component number of multiple coupled variables
   * @return Pointer to the desired variable
   */
  MooseVariable *getVar(const std::string & var_name, unsigned int comp);

};

/**
 * Enhances Coupleable interface to also couple the values from neighbor elements
 *
 */
class NeighborCoupleable : public Coupleable
{
public:
  /**
   * Constructing the object
   * @param parameters Parameters that come from constructing the object
   * @param nodal true if we need to couple with nodal values, otherwise false
   */
  NeighborCoupleable(InputParameters & parameters, bool nodal);

  virtual ~NeighborCoupleable();

  // neighbor
  virtual VariableValue & coupledNeighborValue(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledNeighborValueOld(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledNeighborValueOlder(const std::string & var_name, unsigned int comp = 0);

  virtual VariableGradient & coupledNeighborGradient(const std::string & var_name, unsigned int comp = 0);
  virtual VariableGradient & coupledNeighborGradientOld(const std::string & var_name, unsigned int comp = 0);
  virtual VariableGradient & coupledNeighborGradientOlder(const std::string & var_name, unsigned int comp = 0);

  virtual VariableSecond & coupledNeighborSecond(const std::string & var_name, unsigned int i = 0);
};

/**
 * Interface for objects that needs coupling capabilities
 *
 */
class ScalarCoupleable
{
public:
  ScalarCoupleable(InputParameters & parameters);
  virtual ~ScalarCoupleable();

  /**
   * Get the list of coupled scalar variables
   * @return The list of coupled variables
   */
  const std::vector<MooseVariableScalar *> & getCoupledMooseScalarVars() { return _coupled_moose_scalar_vars; }

protected:
  /**
   * Returns true if a variables has been coupled_as name.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  virtual bool isCoupledScalar(const std::string & var_name, unsigned int i = 0);

  virtual unsigned int coupledScalar(const std::string & var_name, unsigned int comp = 0);

  virtual VariableValue & coupledScalarValue(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledScalarValueOld(const std::string & var_name, unsigned int comp = 0);

  virtual VariableValue & coupledScalarDot(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledScalarDotDu(const std::string & var_name, unsigned int comp = 0);

protected:
  /// Coupled vars whose values we provide
  std::map<std::string, std::vector<MooseVariableScalar *> > _coupled_scalar_vars;
  std::vector<MooseVariableScalar *> _coupled_moose_scalar_vars;
  /// True if implicit value is required
  bool _sc_is_implicit;

  MooseVariableScalar *getScalarVar(const std::string & var_name, unsigned int comp);
};

#endif /* COUPLEABLE_H */
