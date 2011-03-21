#ifndef DIRICHLETBC_H
#define DIRICHLETBC_H

#include "NodalBC.h"

class DirichletBC : public NodalBC
{
public:
  DirichletBC(const std::string & name, InputParameters parameters);
  virtual ~DirichletBC();

protected:
  virtual Real computeQpResidual();

  Real _value;
};

template<>
InputParameters validParams<DirichletBC>();

#endif /* DIRICHLETBC_H_ */
