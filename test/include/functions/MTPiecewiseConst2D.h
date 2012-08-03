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

#ifndef MTPIECEWISECONST2D_H
#define MTPIECEWISECONST2D_H

#include "Function.h"

class MTPiecewiseConst2D;

template<>
InputParameters validParams<MTPiecewiseConst2D>();

class MTPiecewiseConst2D : public Function
{
  public:
      MTPiecewiseConst2D(const std::string & name, InputParameters parameters);

        virtual Real value(Real t, const Point & p);
};

#endif //MTPIECEWISECONST2D_H
