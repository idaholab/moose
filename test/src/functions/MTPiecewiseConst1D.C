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

#include "MTPiecewiseConst1D.h"


template<>
InputParameters validParams<MTPiecewiseConst1D>()
{
    InputParameters params = validParams<Function>();
        return params;
}

MTPiecewiseConst1D::MTPiecewiseConst1D(const std::string & name, InputParameters parameters) :
      Function(name, parameters)
{}

Real
MTPiecewiseConst1D::value(Real /*t*/, const Point & p)
{
    Real val = 0;
    Real x = p(0);
    if      (x>=-0.75 && x<-0.50)val=1.0;
    else if (x>=-0.25 && x< 0.25)val=1.0;
    else if (x>= 0.50 && x< 0.75)val=1.0;
    else val=0.1;
    return val; // p(0) == x
}
