//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "DiffusionHDGKernel.h"
#include "NonADFunctorInterface.h"

#include <vector>

/**
 * Weakly imposes Dirichlet boundary conditions for a hybridized discretization of diffusion
 */
class DiffusionHDGDirichletBC : public IntegratedBC, public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  DiffusionHDGDirichletBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void jacobianSetup() override;
  virtual void initialSetup() override;

protected:
  /**
   * Weakly imposes a Dirichlet condition for the scalar field in the vector (gradient) equation
   */
  void vectorDirichletResidual(const Moose::Functor<Real> & dirichlet_value,
                               const MooseArray<Real> & JxW_face,
                               const QBase & qrule_face,
                               const MooseArray<Point> & normals,
                               const Elem * const current_elem,
                               const unsigned int current_side,
                               const MooseArray<Point> & q_point_face,
                               DenseVector<Number> & vector_re);

  /**
   * Weakly imposes a Dirichlet condition for the scalar field in the scalar field equation
   */
  void scalarDirichletResidual(const MooseArray<Gradient> & vector_sol,
                               const MooseArray<Number> & scalar_sol,
                               const Moose::Functor<Real> & dirichlet_value,
                               const MooseArray<Real> & JxW_face,
                               const QBase & qrule_face,
                               const MooseArray<Point> & normals,
                               const Elem * const current_elem,
                               const unsigned int current_side,
                               const MooseArray<Point> & q_point_face,
                               DenseVector<Number> & scalar_re);

  /**
   * Computes the Jacobian for a Dirichlet condition for the scalar field in the scalar field
   * equation
   */
  void scalarDirichletJacobian(const MooseArray<Real> & JxW_face,
                               const QBase & qrule_face,
                               const MooseArray<Point> & normals,
                               DenseMatrix<Number> & scalar_vector_jac,
                               DenseMatrix<Number> & scalar_scalar_jac);

  /**
   * Creates residuals corresponding to the weak form (v, \hat{u}), or stated simply this routine
   * can be used to drive Lagrange multiplier values on the boundary to zero. This should be used on
   * boundaries where there are Dirichlet conditions for the primal variables such that there is no
   * need for the Lagrange multiplier variables
   */
  void createIdentityResidual(const MooseArray<std::vector<Real>> & phi,
                              const MooseArray<Number> & sol,
                              DenseVector<Number> & re);

  /**
   * As above, but for the Jacobians
   */
  void createIdentityJacobian(const MooseArray<std::vector<Real>> & phi, DenseMatrix<Number> & ke);

  virtual Real computeQpResidual() override { mooseError("this will never be called"); }

  declareDiffusionMembers;

private:
  /// Functor computing the Dirichlet boundary value
  const Moose::Functor<Real> & _dirichlet_val;

  /// A cache variable to prevent multiple computations of Jacobians
  unsigned int _my_side;
};
