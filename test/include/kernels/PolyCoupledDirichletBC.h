#include "BoundaryCondition.h"

#ifndef POLYCOUPLEDDIRICHLETBC_H
#define POLYCOUPLEDDIRICHLETBC_H

class PolyCoupledDirichletBC;

template<>
InputParameters validParams<PolyCoupledDirichletBC>();

class PolyCoupledDirichletBC : public BoundaryCondition
{
public:
  
  PolyCoupledDirichletBC(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
    
  virtual ~PolyCoupledDirichletBC(){}

protected:
  virtual Real computeQpResidual();

private:
  Real _value; //Multiplier on the boundary 
  
};

#endif //POLYCOUPLEDDIRICHLETBC_H
