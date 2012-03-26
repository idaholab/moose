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

#ifndef PARSEDGRADFUNCTION_H
#define PARSEDGRADFUNCTION_H

#include "Function.h"

#include "MooseParsedFunction.h"

class MooseParsedGradFunction;

template<>
InputParameters validParams<MooseParsedGradFunction>();

/**
 * This class is similar to ParsedFunction except it also supports returning the
 * gradient of the function.
 *
 * Documentation for the Function Parser can be found at:
 * http://warp.povusers.org/FunctionParser/fparser.html
 */
class MooseParsedGradFunction : public MooseParsedFunction
{
public:
  MooseParsedGradFunction(const std::string & name, InputParameters parameters);

  virtual ~MooseParsedGradFunction() {}

  virtual RealGradient gradient(Real t, const Point & pt);

protected:
    ParsedFunction<Real> _grad_function;
};

#endif //PARSEDGRADFUNCTION_H

