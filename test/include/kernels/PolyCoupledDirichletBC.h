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

#ifndef POLYCOUPLEDDIRICHLETBC_H
#define POLYCOUPLEDDIRICHLETBC_H

class PolyCoupledDirichletBC;

template<>
InputParameters validParams<PolyCoupledDirichletBC>();

class PolyCoupledDirichletBC : public BoundaryCondition
{
public:
  
  PolyCoupledDirichletBC(const std::string & name, InputParameters parameters);
    
  virtual ~PolyCoupledDirichletBC(){}

protected:
  virtual Real computeQpResidual();

private:
  Real _value; //Multiplier on the boundary 
  
};

#endif //POLYCOUPLEDDIRICHLETBC_H
