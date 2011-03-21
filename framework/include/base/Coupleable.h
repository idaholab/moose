#ifndef COUPLEABLE_H_
#define COUPLEABLE_H_

#include "Variable.h"
#include "InputParameters.h"

class Coupleable
{
public:
  Coupleable(InputParameters & parameters);

protected:
  virtual unsigned int getCoupled(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & getCoupledValue(const std::string & var_name, unsigned int comp = 0);

  virtual VariableValue & getCoupledNodalValue(const std::string & var_name, unsigned int comp = 0);

  std::map<std::string, std::vector<Moose::Variable *> > _coupled_vars;
};

#endif /* COUPLEABLE_H_ */
