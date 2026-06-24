//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarKernelBase.h"
#include "BlockRestrictable.h"
#include "Coupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "MaterialPropertyInterface.h"
#include "ADFunctorInterface.h"

/**
 * Base class for kernels that integrate a scalar variable residual over elements.
 * Unlike ScalarKernel (which assembles globally), this class participates in the
 * element loop so that it can couple to field variables evaluated at quadrature points.
 * Register concrete subclasses under the [ScalarKernels] input block.
 */
class ElementADScalarKernel : public ScalarKernelBase,
                               public BlockRestrictable,
                               public Coupleable,
                               public MooseVariableDependencyInterface,
                               public MaterialPropertyInterface,
                               public ADFunctorInterface
{
public:
  static InputParameters validParams();

  ElementADScalarKernel(const InputParameters & parameters);

  // Called once globally before computeResidual; no-op here because assembly
  // happens per-element from the element loop.
  void reinit() override {}

  // These are no-ops: element scalar kernels are assembled by the element loop
  // threads via computeResidualOnElement / computeJacobianOnElement.
  void computeResidual() override {}
  void computeJacobian() override {}
  void computeResidualAndJacobian() override {}

  /// Called per-element by ComputeResidualThread
  void computeResidualOnElement();

  /// Called per-element by ComputeJacobianThread
  void computeJacobianOnElement();

  /// Called per-element by ComputeResidualAndJacobianThread
  void computeResidualAndJacobianOnElement();

protected:
  /// User implements the scalar residual at quadrature point _qp for DOF _h
  virtual ADReal computeScalarQpResidual() = 0;

  /// Optional per-quadrature-point setup hook (called before the _h loop)
  virtual void initScalarQpResidual() {}

  /// Accumulates AD residuals into _scalar_residuals for Jacobian extraction
  void computeScalarResidualsForJacobian();

  /// Current element (set by the element loop thread)
  const Elem * const & _current_elem;

  /// Quadrature weights * detJ
  const MooseArray<Real> & _JxW;

  /// Coordinate transformation factor (e.g. 2*pi*r for RZ)
  const MooseArray<Real> & _coord;

  /// Active quadrature rule
  const QBase * const & _qrule;

  /// Current quadrature point index
  unsigned int _qp;

  /// Current scalar DOF index (iterates 0.._k_order-1)
  unsigned int _h;

  /// Number of scalar DOFs (order of the scalar variable)
  const unsigned int _k_order;

  /// AD residual storage; derivatives here drive Jacobian extraction
  std::vector<ADReal> _scalar_residuals;
};
