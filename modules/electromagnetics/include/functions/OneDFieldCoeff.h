#ifndef ONEDFIELDCOEFF_H
#define ONEDFIELDCOEFF_H

#include "Function.h"
#include "FunctionInterface.h"

class OneDFieldCoeff;

template <>
InputParameters validParams<OneDFieldCoeff>();

class OneDFieldCoeff : public Function, public FunctionInterface
{
public:
  OneDFieldCoeff(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

private:
  Real _theta;

  const Function & _epsR;

  const Function & _InverseMuR;
};

#endif // ONEDFIELDCOEFF_H
