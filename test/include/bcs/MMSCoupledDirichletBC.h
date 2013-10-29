#ifndef MMSCOUPLEDDIRICHLETBC_H_
#define MMSCOUPLEDDIRICHLETBC_H_

#include "NodalBC.h"

class MMSCoupledDirichletBC;

template<>
InputParameters validParams<MMSCoupledDirichletBC>();

class MMSCoupledDirichletBC : public NodalBC
{
public:
  MMSCoupledDirichletBC(const std::string & name, InputParameters parameters);
  virtual ~MMSCoupledDirichletBC(){}

protected:
  virtual Real computeQpResidual();

  Real _value; //Multiplier on the boundary
  unsigned int _mesh_dimension;
};

#endif //MMSCOUPLEDDIRICHLETBC_H_
