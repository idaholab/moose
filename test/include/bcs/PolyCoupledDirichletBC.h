#ifndef POLYCOUPLEDDIRICHLETBC_H
#define POLYCOUPLEDDIRICHLETBC_H

#include "NodalBC.h"

class PolyCoupledDirichletBC;

template<>
InputParameters validParams<PolyCoupledDirichletBC>();

class PolyCoupledDirichletBC : public NodalBC
{
public:
  PolyCoupledDirichletBC(const std::string & name, InputParameters parameters);

  virtual ~PolyCoupledDirichletBC(){}

protected:
  virtual Real computeQpResidual();

  Real _value; //Multiplier on the boundary
};

#endif //POLYCOUPLEDDIRICHLETBC_H
