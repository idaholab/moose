#ifndef INITIALCONDITION_H_
#define INITIALCONDITION_H_

#include "Object.h"

// System includes
#include <string>

// libMesh
#include "point.h"
#include "vector_value.h"

//forward declarations
class InitialCondition;

template<>
InputParameters validParams<InitialCondition>();

/**
 * InitialConditions are objects that set the initial value of variables.
 */
class InitialCondition : public Object
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   */
  InitialCondition(const std::string & name,
                   InputParameters parameters);

  virtual ~InitialCondition();

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
  virtual RealGradient gradient(const Point & /*p*/) { return RealGradient(); };

protected:
  std::string _var_name;
};

#endif //INITIALCONDITION_H
