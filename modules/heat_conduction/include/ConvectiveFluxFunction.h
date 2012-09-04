#ifndef CONVECTIVEFLUXFUNCTION_H
#define CONVECTIVEFLUXFUNCTION_H

#include "IntegratedBC.h"

class ConvectiveFluxFunction : public IntegratedBC
{
public:

  ConvectiveFluxFunction(const std::string & name, InputParameters parameters);
  virtual ~ConvectiveFluxFunction() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Function & _T_infinity;
  const Real _coefficient;
};

template<>
InputParameters validParams<ConvectiveFluxFunction>();

#endif //CONVECTIVEFLUXFUNCTION_H
