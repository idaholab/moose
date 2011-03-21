#ifndef COUPLED_H_
#define COUPLED_H_

#include "Kernel.h"

class Coupled : public Kernel
{
public:
  Coupled(const std::string & name, InputParameters parameters);
  virtual ~Coupled();

protected:

  virtual Real computeQpResidual();

  VariableValue & _v;
};

template<>
InputParameters validParams<Coupled>();

#endif /* COUPLED_H_ */
