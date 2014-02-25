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

#include "MTPiecewiseConst3D.h"


template<>
InputParameters validParams<MTPiecewiseConst3D>()
{
    InputParameters params = validParams<Function>();
        return params;
}

MTPiecewiseConst3D::MTPiecewiseConst3D(const std::string & name, InputParameters parameters) :
      Function(name, parameters)
{}

Real
MTPiecewiseConst3D::value(Real /*t*/, const Point & p)
{
    Real val = 0;
    Real x = p(0);
    Real y = p(1);
    Real z = p(2);
    if      (x>=-0.75 && x<-0.50)val+=1.0/3.0;
    else if (x>=-0.25 && x< 0.25)val+=1.0/3.0;
    else if (x>= 0.50 && x< 0.75)val+=1.0/3.0;

    if      (y>=-0.75 && y<-0.50)val+=1.0/3.0;
    else if (y>=-0.25 && y< 0.25)val+=1.0/3.0;
    else if (y>= 0.50 && y< 0.75)val+=1.0/3.0;

    if      (z>=-0.75 && z<-0.50)val+=1.0/3.0;
    else if (z>=-0.25 && z< 0.25)val+=1.0/3.0;
    else if (z>= 0.50 && z< 0.75)val+=1.0/3.0;

    return val; // p(0) == x
}
