#ifndef PEC_H
#define PEC_H

#include "NodalNormalBC.h"

class PEC;

template <>
InputParameters validParams<PEC>();

/**
 *
 */
class PEC : public NodalNormalBC
{
public:
  PEC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  const VariableValue & _coupled_val_0;
  const VariableValue & _coupled_val_1;
  const VariableValue & _coupled_val_2;

  int _component;
};

#endif // PEC_H
