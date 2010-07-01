#ifndef EXAMPLEIMPLICITEULER
#define EXAMPLEIMPLICITEULER

#include "ImplicitEuler.h"

class ExampleImplicitEuler;

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

  MaterialProperty<Real> & _time_coefficient;
};

#endif //EXAMPLEIMPLICITEULER
