#pragma once

#include "VectorVariableFromComponentsAux.h"

/**
 * Set 3 standard variables from a vector variable.
 */
class VectorVariableToComponentsAux : public VectorVariableFromComponentsAux
{
public:
  static InputParameters validParams();

  VectorVariableToComponentsAux(const InputParameters & parameters);

protected:
  virtual void compute() override;
};
