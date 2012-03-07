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

  virtual ~Coupleable();

  /**
   * Get the list of coupled variables
   * @return The list of coupled variables
   */
  std::map<std::string, std::vector<MooseVariable *> > & getCoupledVars() { return _coupled_vars; }

protected:
  /**
   * Returns true if a variables has been coupled_as name.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  virtual bool isCoupled(const std::string & var_name, unsigned int i = 0);

  /**
   * Number of coupled components
   * @param var_name Name of the variable
   * @return number of components this variable has (usually 1)
   */
  unsigned int coupledComponents(const std::string & var_name);

  virtual unsigned int coupled(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledValue(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledValueOld(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledValueOlder(const std::string & var_name, unsigned int comp = 0);

  virtual VariableGradient & coupledGradient(const std::string & var_name, unsigned int comp = 0);
  virtual VariableGradient & coupledGradientOld(const std::string & var_name, unsigned int comp = 0);
  virtual VariableGradient & coupledGradientOlder(const std::string & var_name, unsigned int comp = 0);

  virtual VariableSecond & coupledSecond(const std::string & var_name, unsigned int i = 0);
  virtual VariableSecond & coupledSecondOld(const std::string & var_name, unsigned int i = 0);
  virtual VariableSecond & coupledSecondOlder(const std::string & var_name, unsigned int i = 0);

  virtual VariableValue & coupledDot(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledDotDu(const std::string & var_name, unsigned int comp = 0);

protected:
  std::map<std::string, std::vector<MooseVariable *> > _coupled_vars;   ///< Coupled vars whose values we provide
  bool _nodal;                                                          ///< true if we provide coupling to nodal values

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
  std::map<std::string, std::vector<MooseVariableScalar *> > _coupled_scalar_vars;   ///< Coupled vars whose values we provide

  MooseVariableScalar *getScalarVar(const std::string & var_name, unsigned int comp);
};

#endif /* COUPLEABLE_H */
