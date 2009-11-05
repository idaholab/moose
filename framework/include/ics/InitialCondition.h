#ifndef INITIALCONDITION_H
#define INITIALCONDITION_H

#include "Moose.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

/**
 * InitialConditions are objects that set the initial value of variables.
 */
class InitialCondition
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  InitialCondition(std::string name,
                   Parameters parameters,
                   std::string var_name);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point & p) = 0;

  /**
   * The graident of the variable at a point.
   *
   * This is optional.  Note that if you are using C1 continuous elements you will
   * want to use an initial condition that defines this!
   */
  virtual RealGradient gradient(const Point & p){ return RealGradient(); };

private:
  std::string _name;
  Parameters _parameters;
  std::string _var_name;
};

#endif //INITIALCONDITION_H
