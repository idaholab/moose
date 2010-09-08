#ifndef EXAMPLEIMPLICITEULER
#define EXAMPLEIMPLICITEULER

#include "ImplicitEuler.h"

// Forward Declarations
class ExampleImplicitEuler;

template<>
InputParameters validParams<ExampleImplicitEuler>();

class ExampleImplicitEuler : public ImplicitEuler
{
public:

  ExampleImplicitEuler(const std::string & name,
                       MooseSystem &sys,
                       InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  Real _time_coefficient;
};
#endif //EXAMPLEIMPLICITEULER
