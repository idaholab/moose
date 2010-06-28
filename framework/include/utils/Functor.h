#include "fparser.h"

#include <iostream>
#include <string>
#include <map>

#include "libmesh_common.h" // to get typedef Real

/**
 * This class is used to evaluate symbolic equations passed in to Moose through
 * the input file.
 * 
 * Documentation for the Function Parser cab be found at:
 * http://warp.povusers.org/FunctionParser/fparser.html
 */
class Functor
{
  public:
    /**
     * Create a Functor object and initialize the underlying Function Parser.
     *
     * @param equation The actual equation, following the Function Parser for
     *        C++ 4.2" syntax. All basic operators and mathematical functions
     *        are supported.
     * @param vars The variables used in this equation, excluding t, x, y, and z
     */
    Functor( std::string equation, std::vector<std::string> vars = std::vector<std::string>(0) );

    ~Functor();

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
    Real & getVarAddr( std::string var );

    /**
     * Evaluate the equation at the given location. For 1-D and 2-D equations
     * x and y are optional.
     * TODO: override for 1-D and 2-D cases so we don't have to copy 0.0 into
     *       the array every time we evaluate for z and y.
     */
    Real operator()(Real t, Real x, Real y = 0, Real z = 0);

  protected:
    /**
     * Override this method if you want to make your own Functor with custom
     * constants available to the end user. In the base class pi and e are
     * defined. This method is called when the FunctionParser object is being
     * constructed but before it parses the input equation.
     */
    void defineConstants();

  private:
    FunctionParser _parser;
    std::vector<Real> _vars;
    std::map<std::string, Real*> _var_map;
};
