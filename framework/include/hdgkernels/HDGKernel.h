//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class HDGAssemblyHelper;

/**
 * Base kernel for hybridized finite element formulations.
 *
 * Derived kernels return an assembly helper from @ref hdgHelper when they use the common
 * helper-driven assembly. Kernels with their own assembly implementation explicitly return
 * `nullptr`.
 */
class HDGKernel : public Kernel
{
public:
  static InputParameters validParams();

  HDGKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;
  virtual void jacobianSetup() override;
  virtual void computeResidualOnSide();
  virtual void computeJacobianOnSide();
  virtual void computeResidualAndJacobianOnSide();

  virtual std::set<std::string> additionalROVariables() override;
  virtual const std::unordered_set<unsigned int> & getMatPropDependencies() const override;
  virtual bool getMaterialPropertyCalled() const override;

protected:
  /**
   * Returns the helper used for common HDG assembly, or `nullptr` when the derived kernel performs
   * its assembly manually.
   */
  virtual HDGAssemblyHelper * hdgHelper() = 0;
  const HDGAssemblyHelper * hdgHelper() const;

  virtual Real computeQpResidual() override { mooseError("this should never be called"); }

  /// Assembles the helper-driven element-interior residual data.
  void compute();

  /// Assembles the helper-driven face residual data.
  void computeOnSide();

  /// The face quadrature rule
  const QBase * const & _qrule_face;

  /// The physical locations of the quadrature points on the face
  const MooseArray<Point> & _q_point_face;

  /// transformed Jacobian weights on the current element face
  const MooseArray<Real> & _JxW_face;

  /// face normals
  const MooseArray<Point> & _normals;

  /// Current side element
  const Elem * const & _current_side_elem;

  /// Used to assemble the complete AD Jacobian only once per element.
  const Elem * _hdg_cached_elem;
};

inline const HDGAssemblyHelper *
HDGKernel::hdgHelper() const
{
  return const_cast<HDGKernel *>(this)->hdgHelper();
}
