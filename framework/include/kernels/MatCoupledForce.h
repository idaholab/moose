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
  const unsigned int _n_coupled;
  const bool _coupled_props;
  const std::vector<unsigned int> _v_var;
  const std::vector<const VariableValue *> _v;
  const std::vector<Real> _coef;
  std::map<unsigned int, unsigned int> _v_var_to_index;
  std::vector<const MaterialProperty<Real> *> _mat_props;
};
