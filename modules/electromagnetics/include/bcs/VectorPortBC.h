#pragma once

#include "VectorIntegratedBC.h"
#include <complex>

class VectorPortBC : public VectorIntegratedBC
{
public:
  static InputParameters validParams();

  VectorPortBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const Function & _beta;

  MooseEnum _component;

  const VectorVariableValue & _coupled_val;
  unsigned int _coupled_var_num;

  const Function & _inc_real;
  const Function & _inc_imag;

  std::complex<double> _jay;
};
