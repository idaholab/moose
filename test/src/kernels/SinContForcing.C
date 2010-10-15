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

#include "SinContForcing.h"

const Real SinContForcing::_x_center=4;
const Real SinContForcing::_y_center=6;
const Real SinContForcing::_area_of_influence=3;
const Real SinContForcing::_x_min=_x_center-_area_of_influence;
const Real SinContForcing::_x_max=_x_center+_area_of_influence;
const Real SinContForcing::_y_min=_y_center-_area_of_influence;
const Real SinContForcing::_y_max=_y_center+_area_of_influence;

template<>
InputParameters validParams<SinContForcing>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

SinContForcing::SinContForcing(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters)
{}

Real
SinContForcing::computeQpResidual()
{
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);

  if (x >= _x_min && x <= _x_max && y >= _y_min && y <= _y_max)
    return -_test[_i][_qp]*std::exp(-(((x-_x_center)*(x-_x_center))/2.0+((y-_y_center)*(y-_y_center))/2.0));
  else
    return 0;
}

