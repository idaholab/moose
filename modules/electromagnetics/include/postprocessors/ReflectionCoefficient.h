#ifndef REFLECTIONCOEFFICIENT_H
#define REFLECTIONCOEFFICIENT_H

#include "SidePostprocessor.h"
#include "MooseVariableInterface.h"

class ReflectionCoefficient;

template <>
InputParameters validParams<ReflectionCoefficient>();

class ReflectionCoefficient : public SidePostprocessor, public MooseVariableInterface
{
public:
  ReflectionCoefficient(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual Real computeReflection();

  const VariableValue & _u;

  unsigned int _qp;

private:
  Function & _incoming_real;

  Function & _incoming_imag;

  Function & _reflected_real;

  Function & _reflected_imag;

  Real _R;

  const VariableValue & _coupled_imag;
};

#endif // REFLECTIONCOEFFICIENT_H
