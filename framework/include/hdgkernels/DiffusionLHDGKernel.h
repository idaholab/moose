//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HDGKernel.h"
#include "DiffusionLHDGAssemblyHelper.h"

/**
 * Implements the diffusion equation for a hybridized discretization
 */
class DiffusionLHDGKernel : public HDGKernel, public DiffusionLHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  DiffusionLHDGKernel(const InputParameters & params);
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualOnSide() override;
  virtual void computeJacobianOnSide() override;
  virtual void initialSetup() override;
  virtual void jacobianSetup() override;

  virtual std::set<std::string> additionalVariablesCovered() override;

protected:
  /// optional source
  const Moose::Functor<Real> & _source;

  /// The face quadrature rule
  const QBase * const & _qrule_face;

  /// The physical locations of the quadrature points on the face
  const MooseArray<Point> & _q_point_face;

  /// transformed Jacobian weights on the current element face
  const MooseArray<Real> & _JxW_face;

  /// face normals
  const MooseArray<Point> & _normals;
};
