#include "Kernel.h"
#include "Functor.h"

#ifndef FORCINGFUNCTION_H
#define FORCINGFUNCTION_H

//Forward Declarations
class ForcingFunction;

template<>
InputParameters validParams<ForcingFunction>();

/**
 * Define the Kernel for a forcing function that looks like:
 *
 * test function * forcing function
 */
class ForcingFunction : public Kernel
{
public:

  ForcingFunction(std::string name,
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
