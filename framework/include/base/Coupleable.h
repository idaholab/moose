#ifndef COUPLEABLE_H
#define COUPLEABLE_H

#include "MooseVariable.h"
#include "InputParameters.h"


class Coupleable
{
public:
  Coupleable(InputParameters & parameters);

  std::map<std::string, std::vector<MooseVariable *> > & getCoupledVars() { return _coupled_vars; }

protected:
  /**
   * Returns true if a variables has been coupled_as name.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  virtual bool isCoupled(const std::string & varname, unsigned int i = 0);

  virtual unsigned int getCoupled(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & getCoupledValue(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & getCoupledValueOld(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & getCoupledValueOlder(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & getCoupledDot(const std::string & var_name, unsigned int comp = 0);

  virtual VariableGradient & getCoupledGradient(const std::string & var_name, unsigned int comp = 0);
  virtual VariableGradient & getCoupledGradientOld(const std::string & var_name, unsigned int comp = 0);
  virtual VariableGradient & getCoupledGradientOlder(const std::string & var_name, unsigned int comp = 0);

  virtual VariableValue & getCoupledNodalValue(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & getCoupledNodalValueOld(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & getCoupledNodalValueOlder(const std::string & var_name, unsigned int comp = 0);

  std::map<std::string, std::vector<MooseVariable *> > _coupled_vars;
};

#endif /* COUPLEABLE_H */
