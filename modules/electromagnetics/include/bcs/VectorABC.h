#ifndef VECTORABC_H
#define VECTORABC_H

#include "VectorIntegratedBC.h"

class VectorABC;

template <>
InputParameters validParams<VectorABC>();

/**
 *
 */
class VectorABC : public VectorIntegratedBC
{
public:
  VectorABC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  Function & _beta;

  MooseEnum _component;

  const VectorVariableValue & _coupled_val;
};

#endif // VECTORABC_H
