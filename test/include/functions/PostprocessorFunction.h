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

#ifndef POSTPROCESSORFUNCTION_H
#define POSTPROCESSORFUNCTION_H

#include "Function.h"

class PostprocessorFunction;

template<>
InputParameters validParams<PostprocessorFunction>();

class PostprocessorFunction : public Function
{
public:
  PostprocessorFunction(const std::string & name, InputParameters parameters);

  virtual Real value(Real t, const Point & p);

protected:
  const PostprocessorValue & _pp;
};

#endif //POSTPROCESSORFUNCTION_H
