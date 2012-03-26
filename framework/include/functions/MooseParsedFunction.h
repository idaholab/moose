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

#ifndef MOOSEPARSEDFUNCTION_H
#define MOOSEPARSEDFUNCTION_H

#include "Function.h"

#include <iostream>
#include <string>
#include <map>

#include "parsed_function.h"

class MooseParsedFunction;

template<>
InputParameters validParams<MooseParsedFunction>();

/**
 * This class is used to evaluate symbolic equations passed in to Moose through
 * the input file. It supports symbolic variables that you can change by putting
 * a different value in a reference returned by getVarAddr().
 *
 * Documentation for the Function Parser can be found at:
 * http://warp.povusers.org/FunctionParser/fparser.html
 *
 * This class is currently NOT thread safe. If you are going to be evaluating
 * the same functor simultaneously you must compile fparser in thread safe mode.
 * (Do this probably by editing the fconfig.h file)
 */
class MooseParsedFunction : public Function
{
public:
  /**
   * Created from MooseSystem via the FunctionFactory.
   */
  MooseParsedFunction(const std::string & name, InputParameters parameters);

  virtual ~MooseParsedFunction() {};

  /**
   * Get the address to stick the value of the specified variable in. When you
   * evaluate the functor it uses the passed values for t, x, y, and z, and
   * whatever value you put in the address returned by this method for every
   * other symbolic variables.
   *
   * This method should only need to be called once for each variable when you
   * initialize your Kernel/BC/whatever. The address will stay the same for
   * the entire computation.
   *
   * @param var the name of the variable passed in the constructor.
   */

  inline Real & getVarAddr(const std::string & var) { return _function.getVarAddress(var); }

  /**
   * Evaluate the equation at the given location. For 1-D and 2-D equations
   * x and y are optional.
   */
  virtual Real value(Real t, const Point & pt);

protected:
  /**
   * Override this method if you want to make your own MooseParsedFunction with custom
   * constants available to the end user. In the base class pi and e are
   * defined. This method is called when the FunctionParser object is being
   * initialized in initializeParser before the function is parsed.
   */
  // TODO: This function is currently not implemented
  // void defineConstants(FunctionParser & fp);

  std::vector<Real> _vals;

  ParsedFunction<Real> _function;

private:
  std::map<std::string, Real*> _var_map;
};

#endif //MOOSEPARSEDFUNCTION_H
