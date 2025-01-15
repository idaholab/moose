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
#include "DiffusionIPHDGAssemblyHelper.h"

/**
 * Implements the diffusion equation for a interior penalty hybridized discretization
 */
class DiffusionIPHDGKernel : public Kernel, public DiffusionIPHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  DiffusionIPHDGKernel(const InputParameters & params);
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;
  virtual void jacobianSetup() override;

  virtual std::set<std::string> additionalVariablesCovered() override;

protected:
  /**
   * compute the AD residuals
   */
  void compute();

  virtual Real computeQpResidual() override { mooseError("this will never be called"); }

  /// optional source
  const Moose::Functor<Real> & _source;

  /// Neighbor element pointer
  const Elem * _neigh;

  /// The face quadrature rule
  const QBase * const & _qrule_face;

  /// The physical locations of the quadrature points on the face
  const MooseArray<Point> & _q_point_face;

  /// transformed Jacobian weights on the current element face
  const MooseArray<Real> & _JxW_face;

  /// coordinate transformation on the face
  const MooseArray<Real> & _coord_face;

  /// face normals
  const MooseArray<Point> & _normals;

  /// The current side index
  const unsigned int & _current_side;
};
