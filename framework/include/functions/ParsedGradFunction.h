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
