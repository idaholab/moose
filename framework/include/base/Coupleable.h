#ifndef COUPLEABLE_H
#define COUPLEABLE_H

#include "MooseVariable.h"
#include "InputParameters.h"


class Coupleable
{
public:
  Coupleable(InputParameters & parameters, bool nodal);

  std::map<std::string, std::vector<MooseVariable *> > & getCoupledVars() { return _coupled_vars; }

protected:
  /**
   * Returns true if a variables has been coupled_as name.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  virtual bool isCoupled(const std::string & var_name, unsigned int i = 0);

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

  std::map<std::string, std::vector<MooseVariable *> > _coupled_vars;
  bool _nodal;
};

#endif /* COUPLEABLE_H */
