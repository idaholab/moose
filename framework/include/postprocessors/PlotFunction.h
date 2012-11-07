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

#ifndef PLOTFUNCTION_H
#define PLOTFUNCTION_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class PlotFunction;
class Function;

template<>
InputParameters validParams<PlotFunction>();

/**
 * Plot function of time (i.e. f = f(t))
 */
class PlotFunction : public GeneralPostprocessor
{
public:
  PlotFunction(const std::string & name, InputParameters parameters);
  virtual ~PlotFunction();

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  Function & _func;
  const Point _point;
  const Real _scale_factor;
};



#endif /* PLOTFUNCTION_H */
