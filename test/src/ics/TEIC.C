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

#include "TEIC.h"

template<>
InputParameters validParams<TEIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addParam<double>("t_jump", 1.0, "Time when the jump occurs");
  params.addParam<double>("slope", 1.0, "How steep the jump is");
  return params;
}

TEIC::TEIC(const std::string & name, InputParameters parameters) :
    InitialCondition(name, parameters),
    _t_jump(getParam<Real>("t_jump")),
    _slope(getParam<Real>("slope"))
{
}

Real
TEIC::value(const Point & /*p*/)
{
  Real t = 0.0;
  return atan((t - _t_jump)*libMesh::pi * _slope);
}

  


  
