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
#include "InternalSideFluxBase.h"

/**
 * DG side kernel that assembles finite-volume style fluxes for the
 * 2D shallow water equations using a numerical flux userobject.
 *
 * Variables are the conservative set [h, hu, hv]. The kernel is added
 * once per variable and will assemble residual and full 3x3 Jacobians.
 */
class SWEFVFluxDGKernel : public DGKernel
{
public:
  static InputParameters validParams();

  SWEFVFluxDGKernel(const InputParameters & parameters);
  virtual ~SWEFVFluxDGKernel();

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  std::map<unsigned int, unsigned int> getIndexMapping() const;

  // element-side face values (from reconstruction material)
  const MaterialProperty<Real> & _h1;
  const MaterialProperty<Real> & _hu1;
  const MaterialProperty<Real> & _hv1;

  // neighbor-side face values
  const MaterialProperty<Real> & _h2;
  const MaterialProperty<Real> & _hu2;
  const MaterialProperty<Real> & _hv2;

  // numerical flux user object
  const InternalSideFluxBase & _numerical_flux;

  // variable indices for mapping to [h,hu,hv]
  const unsigned int _h_var;
  const unsigned int _hu_var;
  const unsigned int _hv_var;

  const std::map<unsigned int, unsigned int> _jmap;
  const unsigned int _equation_index;

  // Bathymetry variable (cell-constant MONOMIAL/CONSTANT), coupled as a primary variable
  const VariableValue & _b1_var; // current element value at face
  const VariableValue & _b2_var; // neighbor element value at face
};
