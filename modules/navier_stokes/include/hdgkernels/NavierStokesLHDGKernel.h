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
#include "NavierStokesLHDGAssemblyHelper.h"

#include <vector>

/**
 * Implements the steady incompressible Navier-Stokes equations for a hybridized discretization
 */
class NavierStokesLHDGKernel : public HDGKernel, public NavierStokesLHDGAssemblyHelper
{
public:
  static InputParameters validParams();
  NavierStokesLHDGKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualOnSide() override;
  virtual void computeJacobianOnSide() override;
  virtual void initialSetup() override;
  virtual void jacobianSetup() override;

  virtual std::set<std::string> additionalVariablesCovered() override;

protected:
  // body forces
  const Moose::Functor<Real> & _body_force_x;
  const Moose::Functor<Real> & _body_force_y;
  const Moose::Functor<Real> & _body_force_z;
  std::vector<const Moose::Functor<Real> *> _body_forces;
  const Moose::Functor<Real> & _pressure_mms_forcing_function;

  /// The face quadrature rule
  const QBase * const & _qrule_face;

  /// The physical locations of the quadrature points on the face
  const MooseArray<Point> & _q_point_face;

  /// transformed Jacobian weights on the current element face
  const MooseArray<Real> & _JxW_face;

  /// face normals
  const MooseArray<Point> & _normals;

  /// The current side when doing face evaluations
  const unsigned int & _current_side;
};
