#include "Kernel.h"
#include "Functor.h"

#ifndef USERFORCINGFUNCTION_H
#define USERFORCINGFUNCTION_H

//Forward Declarations
class UserForcingFunction;

template<>
InputParameters validParams<UserForcingFunction>();

/**
 * Define the Kernel for a user defined forcing function that looks like:
 *
 * test function * forcing function
 */
class UserForcingFunction : public Kernel
{
public:

  UserForcingFunction(std::string name,
             MooseSystem &sys,
             InputParameters parameters);

protected:
  /**
   * Evaluate f at the current quadrature point.
   */
  Real f();

  /**
   * Computes test function * forcing function.
   */
  virtual Real computeQpResidual();

  /**
   * Currently just 0 because we are assuming f is not coupled to another variable.
   */
  virtual Real computeQpJacobian();

private:
  Functor _functor;
};
#endif //USERFORCINGFUNCTION_H
