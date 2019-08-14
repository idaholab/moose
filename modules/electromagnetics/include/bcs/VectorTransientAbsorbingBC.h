#pragma once

#include "IntegratedBC.h"
#include <complex>

class VectorTransientAbsorbingBC;

template <>
InputParameters validParams<VectorTransientAbsorbingBC>();

class VectorTransientAbsorbingBC : public VectorIntegratedBC
{
public:
  VectorTransientAbsorbingBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const Function & _admittance;

  MooseEnum _component;

  const VectorVariableValue & _coupled_val;
  unsigned int _coupled_var_num;

  const VectorVariableValue & _u_dot;
  const VectorVariableValue & _coupled_dot;

  const VariableValue & _du_dot_du;
  const VariableValue & _coupled_dot_du;

  // TODO: Add incoming field capability
  // const Function & _inc_real;
  // const Function & _inc_imag;

  std::complex<double> _jay;
};
