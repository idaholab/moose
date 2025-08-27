//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "BoundaryFluxBase.h"

/**
 * Boundary condition for SWE using a boundary flux user object.
 * Maps variables [h, hu, hv] to indices [0,1,2].
 */
class SWEFluxBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  SWEFluxBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  // face values for residual
  const MaterialProperty<Real> & _h1;
  const MaterialProperty<Real> & _hu1;
  const MaterialProperty<Real> & _hv1;

  // cell-average values for Jacobian
  const VariableValue & _h;
  const VariableValue & _hu;
  const VariableValue & _hv;

  // variable indices
  const unsigned int _h_var;
  const unsigned int _hu_var;
  const unsigned int _hv_var;

  const std::map<unsigned int, unsigned int> _jmap;
  const unsigned int _equation_index;

  // boundary flux user object
  const BoundaryFluxBase & _flux;

  // Optional cell-constant bathymetry variable to pass as 4th entry to boundary flux UOs
  const bool _has_b;
  const VariableValue * _b_var_val;
};
