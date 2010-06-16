#include "ImplicitEuler.h"

#ifndef EXAMPLEIMPLICITEULER
#define EXAMPLEIMPLICITEULER

class ExampleImplicitEuler;

/**
 * validParams returns the parameters that this Kernel accepts / needs
 * The actual body of the function MUST be in the .C file.
 */
template<>
InputParameters validParams<ExampleImplicitEuler>();

class ExampleImplicitEuler : public ImplicitEuler
{
public:

  ExampleImplicitEuler(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  MooseArray<Real> & _time_coefficient;
};
#endif //EXAMPLEIMPLICITEULER
