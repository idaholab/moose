#ifndef VECTORPORTBC_H
#define VECTORPORTBC_H

#include "VectorIntegratedBC.h"
#include <complex>

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
  unsigned int _coupled_var_num;

  Function & _inc_real;
  Function & _inc_imag;

  std::complex<double> _jay;
};

#endif // VECTORPORTBC_H
