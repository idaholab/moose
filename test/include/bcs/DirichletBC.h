#ifndef DIRICHLETBC_H_
#define DIRICHLETBC_H_

#include "NodalBC.h"

class DirichletBC : public NodalBC
{
public:
  DirichletBC(const std::string & name, InputParameters parameters);
  virtual ~DirichletBC();

protected:
  virtual Real computeNodeResidual();

  Real _value;
};

template<>
InputParameters validParams<DirichletBC>();

#endif /* DIRICHLETBC_H_ */
