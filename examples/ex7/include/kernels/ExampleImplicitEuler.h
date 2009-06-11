#include "ImplicitEuler.h"

#ifndef EXAMPLEIMPLICITEULER
#define EXAMPLEIMPLICITEULER

class ExampleImplicitEuler : public ImplicitEuler
{
public:

  ExampleImplicitEuler(std::string name,
                Parameters parameters,
                std::string var_name,
                std::vector<std::string> coupled_to=std::vector<std::string>(0),
                std::vector<std::string> coupled_as=std::vector<std::string>(0))
    :ImplicitEuler(name,parameters,var_name,coupled_to,coupled_as)
  {}

  virtual void subdomainSetup();

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  std::vector<Real> * _time_coefficient;
};
#endif //EXAMPLEIMPLICITEULER
