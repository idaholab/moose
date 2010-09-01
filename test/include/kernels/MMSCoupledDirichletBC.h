#include "BoundaryCondition.h"

#ifndef MMSCOUPLEDDIRICHLETBC_H
#define MMSCOUPLEDDIRICHLETBC_H

class MMSCoupledDirichletBC;

template<>
InputParameters validParams<MMSCoupledDirichletBC>();

class MMSCoupledDirichletBC : public BoundaryCondition
{
public:
  
  MMSCoupledDirichletBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
    
  virtual ~MMSCoupledDirichletBC(){}

protected:
  virtual Real computeQpResidual();

private:
  Real _value; //Multiplier on the boundary 
  
};

#endif //MMSCOUPLEDDIRICHLETBC_H
