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

#include "GaussContForcing.h"

template<>
InputParameters validParams<GaussContForcing>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<Real>("amplitude", 1.0, "Aplitude of the bell curve");
  params.addParam<Real>("x_center", 4.0, "Center of the hump in the X direction");
  params.addParam<Real>("y_center", 6.0, "Center of the hump in the Y direction");
  params.addParam<Real>("z_center", 0.0, "Center of the hump in the Z direction");
  params.addParam<Real>("x_spread", 1.0, "Spread of the curve in the x direction (sigma_x)");
  params.addParam<Real>("y_spread", 1.0, "Spread of the curve in the y direction (sigma_y)");
  params.addParam<Real>("z_spread", 1.0, "Spread of the curve in the z direction (sigma_z)");
  return params;
}

GaussContForcing::GaussContForcing(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _amplitude(getParam<Real>("amplitude")),
   _x_center(getParam<Real>("x_center")),
   _y_center(getParam<Real>("y_center")),
   _z_center(getParam<Real>("z_center")),
   _x_spread(getParam<Real>("x_spread")),
   _y_spread(getParam<Real>("y_spread")),
   _z_spread(getParam<Real>("z_spread")),
   _x_min(_x_center-(3.0*_x_spread)),
   _x_max(_x_center+(3.0*_x_spread)),
   _y_min(_y_center-(3.0*_y_spread)),
   _y_max(_y_center+(3.0*_y_spread)),
   _z_min(_z_center-(3.0*_z_spread)),
   _z_max(_z_center+(3.0*_z_spread))
{}

Real
GaussContForcing::computeQpResidual()
{
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);

  if (x >= _x_min && x <= _x_max &&
      y >= _y_min && y <= _y_max &&
      z >= _z_min && z <= _z_max)
    return -_test[_i][_qp]*_amplitude*
      std::exp(-(
                 ((x-_x_center)*(x-_x_center))/(2.0*_x_spread*_x_spread)+
                 ((y-_y_center)*(y-_y_center))/(2.0*_y_spread*_y_spread)+
                 ((z-_z_center)*(z-_z_center))/(2.0*_z_spread*_z_spread)
               ));
  else
    return 0;
}

