/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef COMPUTEQPSOLUTION_H
#define COMPUTEQPSOLUTION_H

#include "MooseArray.h"

#include <vector>

#include "libmesh_common.h"

// Forward Declarations
namespace libMesh
{
  template <class T> class NumericVector;
}


/**
   * Computes the value of soln at the current quadrature point.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSolution(Real & u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<Real> > & phi);

  /**
   * Computes the value of all the soln, gradient and second derivative at the current quadrature point in transient problems.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSolutionAll(MooseArray<Real> & u, MooseArray<Real> & u_old, MooseArray<Real> & u_older,
                            MooseArray<RealGradient> &grad_u,  MooseArray<RealGradient> &grad_u_old, MooseArray<RealGradient> &grad_u_older,
                            MooseArray<RealTensor> &second_u,
                            const NumericVector<Number> & soln, const NumericVector<Number> & soln_old,  const NumericVector<Number> & soln_older,
                            const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                            const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi, const std::vector<std::vector<RealTensor> > & d2phi);

  /**
   * Computes the value of all the soln and gradient at the current quadrature point in transient problems.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSolutionAll(MooseArray<Real> & u, MooseArray<Real> & u_old, MooseArray<Real> & u_older,
                            MooseArray<RealGradient> &grad_u,  MooseArray<RealGradient> &grad_u_old, MooseArray<RealGradient> &grad_u_older,
                            const NumericVector<Number> & soln, const NumericVector<Number> & soln_old,  const NumericVector<Number> & soln_older,
                            const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                            const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi);

  /**
   * Computes the value of all the soln, gradient and second derivative at the current quadrature point in steady state problems.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSolutionAll(MooseArray<Real> & u,
                            MooseArray<RealGradient> &grad_u,
                            MooseArray<RealTensor> &second_u,
                            const NumericVector<Number> & soln,
                            const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                            const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi, const std::vector<std::vector<RealTensor> > & d2phi);

  /**
   * Computes the value of all the soln and gradient at the current quadrature point in steady state problems.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSolutionAll(MooseArray<Real> & u,
                            MooseArray<RealGradient> &grad_u,
                            const NumericVector<Number> & soln,
                            const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                            const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi);

  /**
   * Computes the value of the gradient of soln at the current quadrature point.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpGradSolution(RealGradient & grad_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealGradient> > & dphi);

  /**
   * Computes the value of the second derivative of soln at the current quadrature point.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSecondSolution(RealTensor & second_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealTensor> > & d2phi);

#endif //COMPUTEQPSOLUTION_H
