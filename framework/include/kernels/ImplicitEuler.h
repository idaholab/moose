#ifndef IMPLICITEULER_H_
#define IMPLICITEULER_H_

#include "TimeDerivative.h"

// Forward Declaration
class ImplicitEuler;

template<>
InputParameters validParams<ImplicitEuler>();

class ImplicitEuler : public TimeDerivative
{
public:
  ImplicitEuler(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

};
#endif //IMPLICITEULER_H_
