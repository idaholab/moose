#ifndef VECTORPORTBC_H
#define VECTORPORTBC_H

#include "VectorIntegratedBC.h"

class VectorPortBC;

template <>
InputParameters validParams<VectorPortBC>();

/**
 *
 */
class VectorPortBC : public VectorIntegratedBC
{
public:
  VectorPortBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  Function & _beta;

  MooseEnum _component;

  const VectorVariableValue & _coupled_val;
  unsigned int _coupled_id;

  Function & _inc_real_x;
  Function & _inc_real_y;
  Function & _inc_real_z;

  Function & _inc_imag_x;
  Function & _inc_imag_y;
  Function & _inc_imag_z;
};

#endif // VECTORPORTBC_H
