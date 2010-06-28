#include "Kernel.h"
#include "Functor.h"

#ifndef FORCINGFUNCTION_H
#define FORCINGFUNCTION_H

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
#endif //FORCINGFUNCTION_H
