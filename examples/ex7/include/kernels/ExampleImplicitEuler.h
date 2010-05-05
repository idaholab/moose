#include "ImplicitEuler.h"

#ifndef EXAMPLEIMPLICITEULER
#define EXAMPLEIMPLICITEULER

class ExampleImplicitEuler : public ImplicitEuler
{
public:

  ExampleImplicitEuler(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters);

  virtual void subdomainSetup();

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  std::vector<Real> * _time_coefficient;
};
#endif //EXAMPLEIMPLICITEULER
