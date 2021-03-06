#pragma once

#include "Reaction.h"

class CoeffField : public Reaction
{
public:
  static InputParameters validParams();

  CoeffField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  Real _coefficient;

  const Function & _func;
};
