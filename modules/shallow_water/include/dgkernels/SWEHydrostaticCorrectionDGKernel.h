//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DGKernel.h"

/**
 * Hydrostatic correction DGKernel (Audusse-style) for SWE momentum equations.
 *
 * Adds face contributions to momentum components so that lake-at-rest
 * (eta = const) is preserved exactly with hydrostatic reconstruction.
 *
 * Residual per face on the left (elem) side:
 *   R_mom += 0.5 * g * (h_L^2 - hL*^2) * n_component * test
 * and on the right (neighbor) side the opposite sign with right states.
 *
 * Only acts for momentum variables (hu or hv); returns zero for h.
 */
class SWEHydrostaticCorrectionDGKernel : public DGKernel
{
public:
  static InputParameters validParams();

  SWEHydrostaticCorrectionDGKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  std::map<unsigned int, unsigned int> getIndexMapping() const;

  // face values of depth (reconstruction material)
  const MaterialProperty<Real> & _h1;
  const MaterialProperty<Real> & _h2;

  // Bathymetry variable coupling (cell-constant)
  const VariableValue & _b1_var;
  const VariableValue & _b2_var;

  // coupled variables and mapping
  const unsigned int _h_var;
  const unsigned int _hu_var;
  const unsigned int _hv_var;
  const std::map<unsigned int, unsigned int> _jmap;
  const unsigned int _equation_index; // 0:h, 1:hu, 2:hv

  // gravity (coupled scalar field)
  const VariableValue & _g;
};
