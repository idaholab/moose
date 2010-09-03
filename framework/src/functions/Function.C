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

#include "Function.h"

template<>
InputParameters validParams<Function>()
{
  InputParameters params = validParams<MooseObject>();
  return params;
}

Function::Function(std::string name, MooseSystem & moose_system, InputParameters parameters) :
  MooseObject(name, moose_system, parameters)
{
}

Function::~Function()
{
}

RealGradient 
Function::gradient(Real t, Real x, Real y, Real z)
{
  return RealGradient(0, 0, 0);
}
