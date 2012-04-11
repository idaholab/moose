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

#include "ExampleIC.h"

template<>
InputParameters validParams<ExampleIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("coefficient", "The value of the initial condition");
  return params;
}

ExampleIC::ExampleIC(const std::string & name,
                     InputParameters parameters) :
    InitialCondition(name, parameters),
    _coefficient(getParam<Real>("coefficient"))
{}

Real
ExampleIC::value(const Point & p)
{
  /**
   * _value * x
   * The Point class is defined in libMesh.  The spatial
   * coordinates x,y,z can be accessed individually using
   * the parenthesis operator and a numeric index from 0..2
   */
  return 2.*_coefficient*std::abs(p(0));
}
