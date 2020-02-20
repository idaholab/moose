//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward Declaration
class MatCoupledForce;

template <>
InputParameters validParams<MatCoupledForce>();

/**
 * Represents a right hand side force term of the form
 * Sum_j c_j * m_j * v_j, where c is a vector of real numbers,
 * m_j is a vector of material properties, and v_j is a vector
 * of variables
 */
class MatCoupledForce : public Kernel
{
public:
  static InputParameters validParams();

  MatCoupledForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  unsigned int _n_coupled;
  bool _coupled_props;
  std::vector<unsigned int> _v_var;
  std::vector<const VariableValue *> _v;
  std::vector<Real> _coef;
  std::vector<const MaterialProperty<Real> *> _mat_props;
  std::map<unsigned int, unsigned int> _v_var_to_index;
};
