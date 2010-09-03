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

#include "RandomIC.h"

template<>
InputParameters validParams<RandomIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("min", "Lower bound of the randomly generated values");
  params.addRequiredParam<Real>("max", "Upper bound of the randomly generated values");
  return params;
}

RandomIC::RandomIC(std::string name,
                   MooseSystem & moose_system,
                   InputParameters parameters)
  :InitialCondition(name, moose_system, parameters),
   _min(parameters.get<Real>("min")),
   _max(parameters.get<Real>("max")),
   _range(_max - _min)
{
  mooseAssert(_range > 0.0, "Min > Max for RandomIC!");
}

Real
RandomIC::value(const Point & /*p*/)
{
  //Random number between 0 and 1
  Real rand_num = (Real)rand() / (Real)RAND_MAX;

  //Between 0 and range
  rand_num *= _range;

  //Between min and max
  rand_num += _min;

  return rand_num;
}

  


  
