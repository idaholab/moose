/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
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

#include "ParsedFunction.h"

class ParsedGradFunction;

template<>
InputParameters validParams<ParsedGradFunction>();

/**
 * This class is similar to ParsedFunction except it also supports returning the
 * gradient of the function.
 *
 * Documentation for the Function Parser can be found at:
 * http://warp.povusers.org/FunctionParser/fparser.html
 */
class ParsedGradFunction : public ParsedFunction
{
  public:
    ParsedGradFunction(std::string name, MooseSystem & moose_system, InputParameters parameters);

    virtual RealGradient grad(Real t, Real x, Real y = 0, Real z = 0);

  private:
    FunctionParser _parserx;
    FunctionParser _parsery;
    FunctionParser _parserz;
};

#endif //PARSEDGRADFUNCTION_H
