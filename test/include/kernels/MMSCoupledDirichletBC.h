/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "BoundaryCondition.h"

#ifndef MMSCOUPLEDDIRICHLETBC_H
#define MMSCOUPLEDDIRICHLETBC_H

class MMSCoupledDirichletBC;

template<>
InputParameters validParams<MMSCoupledDirichletBC>();

class MMSCoupledDirichletBC : public BoundaryCondition
{
public:
  
  MMSCoupledDirichletBC(const std::string & name, InputParameters parameters);
    
  virtual ~MMSCoupledDirichletBC(){}

protected:
  virtual Real computeQpResidual();

private:
  Real _value; //Multiplier on the boundary 
  
};

#endif //MMSCOUPLEDDIRICHLETBC_H
