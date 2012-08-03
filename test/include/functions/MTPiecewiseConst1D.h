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

#ifndef MTPIECEWISECONST1D_H
#define MTPIECEWISECONST1D_H

#include "Function.h"

class MTPiecewiseConst1D;

template<>
InputParameters validParams<MTPiecewiseConst1D>();

class MTPiecewiseConst1D : public Function
{
  public:
      MTPiecewiseConst1D(const std::string & name, InputParameters parameters);

        virtual Real value(Real t, const Point & p);
};

#endif //MTPIECEWISECONST1D_H
