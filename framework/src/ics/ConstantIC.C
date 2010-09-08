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

#include "ConstantIC.h"

template<>
InputParameters validParams<ConstantIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.set<Real>("value") = 0.0;
  return params;
}

ConstantIC::ConstantIC(std::string name,
                       MooseSystem & moose_system,
                       InputParameters parameters)
  :InitialCondition(name, moose_system, parameters),
   _value(getParam<Real>("value"))
{}

Real
ConstantIC::value(const Point & /*p*/)
{
  return _value;
}

  


  
