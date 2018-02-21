#ifndef ROBINREFLECTIONBC_H
#define ROBINREFLECTIONBC_H

#include "IntegratedBC.h"

class RobinReflectionBC;

template<>
InputParameters validParams<RobinReflectionBC>();

class RobinReflectionBC : public IntegratedBC
{
public:
  RobinReflectionBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:

  Real _theta;

  const VariableValue & _coupled_val;

  Real _L;

  std::string _num_type;

  Real _k;

  Function & _inverseMuR;

};

#endif // ROBINREFLECTIONBC_H
