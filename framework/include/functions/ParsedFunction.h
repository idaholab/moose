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

#ifndef PARSEDFUNCTION_H
#define PARSEDFUNCTION_H

#include "Function.h"
#include "fparser.h"

#include <iostream>
#include <string>
#include <map>

#include "libmesh_common.h" // to get Real

class ParsedFunction;

template<>
InputParameters validParams<ParsedFunction>();

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
class ParsedFunction : public Function
{
public:
  /**
   * Created from MooseSystem via the FunctionFactory.
   */
  ParsedFunction(const std::string & name, InputParameters parameters);

  virtual ~ParsedFunction();

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
  Real & getVarAddr(std::string var);

  /**
   * Evaluate the equation at the given location. For 1-D and 2-D equations
   * x and y are optional.
   */
  virtual Real value(Real t, const Point & pt);

protected:
  /**
   * Override this method if you want to make your own ParsedFunction with custom
   * constants available to the end user. In the base class pi and e are
   * defined. This method is called when the FunctionParser object is being
   * initialized in initializeParser before the function is parsed.
   */
  void defineConstants(FunctionParser & fp);

  /**
   * Called from the constructor to initialize the underlying Function Parser.
   *
   * @param equation The actual equation, following the "Function Parser for
   *        C++ 4.2" syntax. All basic operators and mathematical functions
   *        are supported.
   * @param vars The variables used in this equation, excluding t, x, y, and z
   * @param vals The values to stick in the variables listed in vars. If you
   *        want to dynamically change the variables this parameter is usually
   *        not used and the values are filled with getVarAddr().
   *
   *        This list may be shorter than the vars list, in which case the vals
   *        will be put in the first `len(vals)` variables. In that case you
   *        would have to set the remaining variables using getVarAddr().
   *
   *        TODO: putting these values as constants would probably be faster
   */
  void initializeParser(FunctionParser & fp, std::string equation,
           std::vector<std::string> vars = std::vector<std::string>(0),
           std::vector<Real> vals = std::vector<Real>(0));

  std::vector<Real> _vals;

private:
  FunctionParser _parser;
  std::map<std::string, Real*> _var_map;
};

#endif //PARSEDFUNCTION_H
