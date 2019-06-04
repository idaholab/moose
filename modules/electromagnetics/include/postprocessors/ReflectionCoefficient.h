#ifndef REFLECTIONCOEFFICIENT_H
#define REFLECTIONCOEFFICIENT_H

#include "SidePostprocessor.h"
#include "MooseVariableInterface.h"

class ReflectionCoefficient;

template <>
InputParameters validParams<ReflectionCoefficient>();

class ReflectionCoefficient : public SidePostprocessor, public MooseVariableInterface<Real>
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
  const VariableValue & _coupled_imag;

  Real _theta;

  Real _length;

  Real _k;

  Real _coeff;

  Real _reflection_coefficient;
};

#endif // REFLECTIONCOEFFICIENT_H
