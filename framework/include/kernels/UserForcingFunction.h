#ifndef USERFORCINGFUNCTION_H
#define USERFORCINGFUNCTION_H

//TODO move to .C file?
#include "Kernel.h"
#include "Function.h"

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

private:
  Function & _func;
};
#endif //USERFORCINGFUNCTION_H
