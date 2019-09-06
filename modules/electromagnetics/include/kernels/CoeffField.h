#pragma once

#include "Reaction.h"

// Forward declarations
class CoeffField;

template <>
InputParameters validParams<CoeffField>();

class CoeffField : public Reaction
{
public:
  CoeffField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  Real _coefficient;

  const Function & _func;
};
